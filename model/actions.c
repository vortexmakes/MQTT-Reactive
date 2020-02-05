/* ------------------------------ File header ------------------------------ */
/* -------------------------- Development history -------------------------- */
/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci  lf@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
#include "mqtt.h"

/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
static const MQTTProtCfg configDft =
{
    60, 5, "publishing_cLient", 400, "date_time", 0
};

/* ---------------------------- Local data types --------------------------- */
typedef struct AppData AppData;
struct AppData
{
    uint8_t *data;
    uint16_t size;
};

typedef uint16_t (*MQTTProtPublish)(AppData *appMsg);

typedef struct MQTTProtCfg MQTTProtCfg;
struct MQTTProtCfg
{
    uint16_t publishTime;    /* in secs */
    uint16_t syncTime;       /* in secs */
    char clientId[23];
    uint16_t keepAlive;      /* in secs */
    char topic[20];
    uint8_t qos;             /* 0, 1 or 2 */
};

typedef struct MqttClient MqttClient;
struct MqttClient
{
    struct mqtt_client client;
    uint8_t sendbuf[2048];  /* sendbuf should be large enough to hold */
                            /* multiple whole mqtt messages */
    uint8_t recvbuf[1024];  /* recvbuf should be large enough any whole */
                            /* mqtt message expected to be received */
    enum MQTTErrors operRes;
    MQTTProtCfg *config;
    MQTTProtPublish publisher;
    const char *errorStr;
    LocalSendAll localSend;
    LocalRecvAll localRecv;
    Queue deferQueue;
    Event *deferQueueStorage[MAX_SIZEOF_DEFERQUEUE];
};

/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static SendEvt evSendObj;
static ConnRefusedEvt evConnRefusedObj;

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
static void
connack_response_callback(enum MQTTConnackReturnCode returnCode)
{
    MqttClient *me;
    Event *evt;

    if (returnCode == MQTT_CONNACK_ACCEPTED)
    {
        evt = (Event *)&evConnAcceptedObj;
    }
    else
    {
        evConnRefusedObj.code = returnCode;    /* it should be dynamic */
        evt = RKH_UPCAST(Event, &evConnRefusedObj);
    }
    postEvent(me, evt);
}

/* ---------------------------- Global functions --------------------------- */
/* ............................ Effect actions ............................. */
void
initialize(MqttClient *const me, Event *consumedEvent)
{
    queue_init(&me->deferQueue, 
               (const void **)me->deferQueueStorage, 
               MAX_SIZEOF_DEFERQUEUE, NULL);

    mqtt_init(&me->client, 0, 
              me->sendbuf, sizeof(me->sendbuf), me->recvbuf, 
              sizeof(me->recvbuf), 0);

    me->client.connack_response_callback = connack_response_callback;
}

void
connect(MqttClient *const me, Event *consumedEvent)
{
    me->operRes = mqtt_connect(&me->client, 
                               me->config->clientId, 
                               NULL, NULL, 0, NULL, NULL, 0, 
                               me->config->keepAlive);
}

void
publish(MqttClient *const me, Event *consumedEvent)
{
    AppData appMsg;
    uint16_t pubTime;

	(void)consumedEvent;

    pubTime = (*me->publisher)(&appMsg);
    if (pubTime != 0)
    {
        me->config->publishTime = pubTime;
    }
    me->operRes = mqtt_publish(&me->client, 
                               me->config->topic, 
                               appMsg.data, 
                               appMsg.size, 
                               (me->config->qos << 1) & 0x06);
}

void 
initRecvAll(MqttClient *const me, Event *consumedEvent)
{
	(void)consumedEvent;

    mqtt_isReconnect(me->client);
    mqtt_initRecvAll();
}

void 
parseRecv(MqttClient *const me, Event *consumedEvent)
{
    ReceivedEvt *evt;

    evt = RKH_DOWNCAST(ReceivedEvt, consumedEvent);
    memcpy(me->client.recv_buffer.curr, evt->buf, evt->size);
    me->localRecv.rv = evt->size;
    mqtt_parseRecv(&me->client, &me->localRecv);
}

void 
recvFail(MqttClient *const me, Event *consumedEvent)
{
	(void)consumedEvent;

    me->localRecv.rv = MQTT_ERROR_SOCKET_ERROR;
    mqtt_recvFail(&me->client, &me->localRecv); /* an error occurred */
}

void 
parseError(MqttClient *const me, Event *consumedEvent)
{
	(void)consumedEvent;

    mqtt_parseError(&me->client, &me->localRecv);
}

void 
noConsumed(MqttClient *const me, Event *consumedEvent)
{
	(void)consumedEvent;

    mqtt_noConsumed(&me->client, &me->localRecv);
}

void 
handleRecvMsg(MqttClient *const me, Event *consumedEvent)
{
	(void)consumedEvent;

    mqtt_handleRecvMsg(&me->client, &me->localRecv);
}

void 
recvMsgError(MqttClient *const me, Event *consumedEvent)
{
	(void)consumedEvent;

    mqtt_recvMsgError(&me->client, &me->localRecv);
}

void 
cleanBuf(MqttClient *const me, Event *consumedEvent)
{
	(void)consumedEvent;

    mqtt_cleanBuf(&me->client, &me->localRecv);
}

void 
initSendAll(MqttClient *const me, Event *consumedEvent)
{
	(void)consumedEvent;

    mqtt_initSendAll(&me->client, &me->localSend);
}

void 
initSendAll(MqttClient *const me, Event *consumedEvent)
{
	(void)consumedEvent;

    mqtt_initSendAll(&me->client, &me->localSend);
}

void 
endSendAll(MqttClient *const me, Event *consumedEvent)
{
	(void)consumedEvent;

    mqtt_endSendAll(&me->client);
}

void 
sendOneMsg(MqttClient *const me, Event *consumedEvent)
{
	(void)consumedEvent;

    mqtt_sendOneMsg(&me->client, &me->localSend);
}

void 
nextSend(MqttClient *const me, Event *consumedEvent)
{
	(void)me;
	(void)consumedEvent;

    mqtt_nextSend(&me->localSend);
}

void 
sendMsgFail(MqttClient *const me, Event *consumedEvent)
{
	(void)consumedEvent;

    me->localSend.tmp = MQTT_ERROR_SOCKET_ERROR;
    mqtt_sendMsgFail(&me->client, &me->localSend);
}

void 
setMsgState(MqttClient *const me, Event *consumedEvent)
{
	(void)consumedEvent;

    mqtt_setMsgState(&me->client, &me->localSend);
}

void 
recall(MqttClient *const me, Event *consumedEvent)
{
    recallEvent(me, &me->deferQueue);
}

void 
defer(MqttClient *const me, Event *consumedEvent)
{
    deferEvent(&me->deferQueue, consumedEvent);
}

/* ............................. Entry actions ............................. */
void 
recvAll(MqttClient *const me, Event *consumedEvent)
{
	(void)consumedEvent;

    ConnectionTopic_publish(&evRecvObj, me);
}

void 
sendAll(MqttClient *const me, Event *consumedEvent)
{
	(void)consumedEvent;

    evSendObj.size = me->localSend.msg->size;
    memcpy(evSendObj.buf, me->localSend.msg->start, me->localSend.msg->size);
    ConnectionTopic_publish(&evSendObj, me);
}

/* ............................. Exit actions .............................. */
/* ................................ Guards ................................. */
rbool_t 
isRecvBufFull(MqttClient *const me, Event *consumedEvent)
{
	(void)me;
	(void)consumedEvent;

    return mqtt_isRecvBufFull(&me->localRecv);
}

rbool_t 
isInitOk(MqttClient *const me, Event *consumedEvent)
{
	(void)me;
	(void)consumedEvent;

    return mqtt_isInitOk(&me->localSend);
}

rbool_t 
isThereMsg(MqttClient *const me, Event *consumedEvent)
{
	(void)me;
	(void)consumedEvent;

    return mqtt_isThereMsg(&me->localSend);
}

rbool_t 
isNotResend(MqttClient *const me, Event *consumedEvent)
{
	(void)me;
	(void)consumedEvent;

    return me->localSend.resend == 0;
}

rbool_t 
isSetMsgStateOk(MqttClient *const me, Event *consumedEvent)
{
	(void)me;
	(void)consumedEvent;

    return mqtt_isSetMsgStateResult(&me->localSend);
}

rbool_t 
isUnpackError(MqttClient *const me, Event *consumedEvent)
{
	(void)me;
	(void)consumedEvent;

    return mqtt_isUnpackError(&me->localRecv);
}

rbool_t 
isConsumed(MqttClient *const me, Event *consumedEvent)
{
	(void)me;
	(void)consumedEvent;

    return mqtt_isConsumed(&me->localRecv);
}

rbool_t 
isNotError(MqttClient *const me, Event *consumedEvent)
{
	(void)me;
	(void)consumedEvent;

    return mqtt_isNotError(&me->localRecv);
}

/* ------------------------------ File footer ------------------------------ */

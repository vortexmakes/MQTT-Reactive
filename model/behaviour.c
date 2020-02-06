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
initialize
===============================================================================
queue_init(&me->deferqueue, 
           (const void **)me->deferQueueStorage, 
           max_sizeof_deferqueue, null);
mqtt_init(&me->client, 0, 
          me->sendbuf, sizeof(me->sendbuf), 
          me->recvbuf, sizeof(me->recvbuf), 0);
me->client.connack_response_callback = connack_response_callback;
===============================================================================

connect
===============================================================================
me->operRes = mqtt_connect(&me->client, me->config->clientId, 
                           NULL, NULL, 0, NULL, NULL, 0, 
                           me->config->keepAlive);
===============================================================================

publish
===============================================================================
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
===============================================================================

initRecvAll
===============================================================================
mqtt_isReconnect(me->client);
mqtt_initRecvAll();
===============================================================================

parseRecv
===============================================================================
ReceivedEvt *evt;

evt = RKH_DOWNCAST(ReceivedEvt, consumedEvent);
memcpy(me->client.recv_buffer.curr, evt->buf, evt->size);
me->localRecv.rv = evt->size;
mqtt_parseRecv(&me->client, &me->localRecv);
===============================================================================

recvFail
===============================================================================
me->localRecv.rv = MQTT_ERROR_SOCKET_ERROR;
mqtt_recvFail(&me->client, &me->localRecv); /* an error occurred */
===============================================================================

parseError
===============================================================================
mqtt_parseError(&me->client, &me->localRecv);
===============================================================================

noConsumed
===============================================================================
mqtt_noConsumed(&me->client, &me->localRecv);
===============================================================================

handleRecvMsg
===============================================================================
mqtt_handleRecvMsg(&me->client, &me->localRecv);
===============================================================================

recvMsgError
===============================================================================
mqtt_recvMsgError(&me->client, &me->localRecv);
===============================================================================

cleanBuf
===============================================================================
mqtt_cleanBuf(&me->client, &me->localRecv);
===============================================================================

initSendAll
===============================================================================
mqtt_initSendAll(&me->client, &me->localSend);
===============================================================================

initSendAll
===============================================================================
mqtt_initSendAll(&me->client, &me->localSend);
===============================================================================

endSendAll
===============================================================================
mqtt_endSendAll(&me->client);
===============================================================================

sendOneMsg
===============================================================================
mqtt_sendOneMsg(&me->client, &me->localSend);
===============================================================================

nextSend
===============================================================================
mqtt_nextSend(&me->localSend);
===============================================================================

sendMsgFail
===============================================================================
me->localSend.tmp = MQTT_ERROR_SOCKET_ERROR;
mqtt_sendMsgFail(&me->client, &me->localSend);
===============================================================================

setMsgState
===============================================================================
mqtt_setMsgState(&me->client, &me->localSend);
===============================================================================

recall
===============================================================================
recallEvent(me, &me->deferQueue);
===============================================================================

defer
===============================================================================
deferEvent(&me->deferQueue, consumedEvent);
===============================================================================

/* ............................. Entry actions ............................. */
recvAll
===============================================================================
ConnectionTopic_publish(&evRecvObj, me);
===============================================================================

sendAll
===============================================================================
evSendObj.size = me->localSend.msg->size;
memcpy(evSendObj.buf, me->localSend.msg->start, me->localSend.msg->size);
ConnectionTopic_publish(&evSendObj, me);
===============================================================================

/* ............................. Exit actions .............................. */
/* ................................ Guards ................................. */
isRecvBufFull
===============================================================================
mqtt_isRecvBufFull(&me->localRecv) == true
===============================================================================

isInitOk
===============================================================================
mqtt_isInitOk(&me->localSend) == true
===============================================================================

isThereMsg
===============================================================================
mqtt_isThereMsg(&me->localSend) == true
===============================================================================

isNotResend
===============================================================================
me->localSend.resend == 0
===============================================================================

isSetMsgStateOk
===============================================================================
mqtt_isSetMsgStateResult(&me->localSend) == true
===============================================================================

isUnpackError
===============================================================================
mqtt_isUnpackError(&me->localRecv) == true
===============================================================================

isConsumed
===============================================================================
mqtt_isConsumed(&me->localRecv) == true
===============================================================================

isNotError
===============================================================================
mqtt_isNotError(&me->localRecv) == true
===============================================================================

/* ------------------------------ File footer ------------------------------ */

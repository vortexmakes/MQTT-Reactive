/* ------------------------------ File header ------------------------------ */
/* -------------------------- Development history -------------------------- */
/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci  lf@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* ----------------------------- Include files ----------------------------- */
/* ----------------------------- Local macros ------------------------------ */
/* ------------------------------- Constants ------------------------------- */
static const MQTTProtCfg configDft =
{
    60, 5, "publishing_cLient", 400, "date_time", 0
};

/* ---------------------------- Local data types --------------------------- */
typedef struct ConnRefusedEvt ConnRefusedEvt;
struct ConnRefusedEvt
{
    RKH_EVT_T evt;
    enum MQTTConnackReturnCode code;
};

typedef struct AppData AppData;
struct AppData
{
    rui8_t *data;
    rui16_t size;
};

typedef rui16_t (*MQTTProtPublish)(AppData *appMsg);

typedef struct MQTTProtCfg MQTTProtCfg;
struct MQTTProtCfg
{
    rui16_t publishTime;    /* in secs */
    rui16_t syncTime;       /* in secs */
    char clientId[23];
    rui16_t keepAlive;      /* in secs */
    char topic[20];
    rui8_t qos;             /* 0, 1 or 2 */
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
};

/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static Queue deferQueue;
static Event *deferQueueStorage[MAX_SIZEOF_DEFERQUEUE];
static SendEvt evSendObj;
static ConnRefusedEvt evConnRefusedObj;
static LocalSendAll localSend;
static LocalRecvAll localRecv;

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
static void
connack_response_callback(enum MQTTConnackReturnCode return_code)
{
    MqttClient *me;
    Event *evt;

    me = RKH_DOWNCAST(MqttClient, mqttProt);
    if (return_code == MQTT_CONNACK_ACCEPTED)
    {
        evt = RKH_UPCAST(Event, &evConnAcceptedObj);
    }
    else
    {
        evConnRefusedObj.code = return_code;    /* it should be dynamic */
        evt = RKH_UPCAST(Event, &evConnRefusedObj);
    }
    postEvent(me, evt);
}

/* ---------------------------- Global functions --------------------------- */
/* ............................ Effect actions ............................. */
void
initialize(...)
{
    queue_init(&deferQueue, 
               (const void **)deferQueueStorage, 
               MAX_SIZEOF_DEFERQUEUE, NULL);

    mqtt_init(&me->client, 0, 
              me->sendbuf, sizeof(me->sendbuf), me->recvbuf, 
              sizeof(me->recvbuf), 0);

    me->client.connack_response_callback = connack_response_callback;
}

void
connect(...)
{
    me->operRes = mqtt_connect(&me->client, 
                               me->config->clientId, 
                               NULL, NULL, 0, NULL, NULL, 0, 
                               me->config->keepAlive);
}

void
publish(...)
{
    AppData appMsg;
    uint16_t pubTime;

	(void)pe;

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
initRecvAll(MqttClient *const me, Event *pe)
{
	(void)pe;

    mqtt_isReconnect(me->client);
    mqtt_initRecvAll();
}

void 
parseRecv(MqttClient *const me, Event *pe)
{
    ReceivedEvt *evt;

    evt = RKH_DOWNCAST(ReceivedEvt, pe);
    memcpy(me->client.recv_buffer.curr, evt->buf, evt->size);
    localRecv.rv = evt->size;
    mqtt_parseRecv(&me->client, &localRecv);
}

void 
recvFail(MqttClient *const me, Event *pe)
{
	(void)pe;

    localRecv.rv = MQTT_ERROR_SOCKET_ERROR;
    mqtt_recvFail(&me->client, &localRecv); /* an error occurred */
}

void 
parseError(MqttClient *const me, Event *pe)
{
	(void)pe;

    mqtt_parseError(&me->client, &localRecv);
}

void 
noConsumed(MqttClient *const me, Event *pe)
{
	(void)pe;

    mqtt_noConsumed(&me->client, &localRecv);
}

void 
handleRecvMsg(MqttClient *const me, Event *pe)
{
	(void)pe;

    mqtt_handleRecvMsg(&me->client, &localRecv);
}

void 
recvMsgError(MqttClient *const me, Event *pe)
{
	(void)pe;

    mqtt_recvMsgError(&me->client, &localRecv);
}

void 
cleanBuf(MqttClient *const me, Event *pe)
{
	(void)pe;

    mqtt_cleanBuf(&me->client, &localRecv);
}

void 
initSendAll(MqttClient *const me, Event *pe)
{
	(void)pe;

    mqtt_initSendAll(&me->client, &localSend);
}

void 
initSendAll(MqttClient *const me, Event *pe)
{
	(void)pe;

    mqtt_initSendAll(&me->client, &localSend);
}

void 
endSendAll(MqttClient *const me, Event *pe)
{
	(void)pe;

    mqtt_endSendAll(&me->client);
}

void 
sendOneMsg(MqttClient *const me, Event *pe)
{
	(void)pe;

    mqtt_sendOneMsg(&me->client, &localSend);
}

void 
nextSend(MqttClient *const me, Event *pe)
{
	(void)me;
	(void)pe;

    mqtt_nextSend(&localSend);
}

void 
sendMsgFail(MqttClient *const me, Event *pe)
{
	(void)pe;

    localSend.tmp = MQTT_ERROR_SOCKET_ERROR;
    mqtt_sendMsgFail(&me->client, &localSend);
}

void 
setMsgState(MqttClient *const me, Event *pe)
{
	(void)pe;

    mqtt_setMsgState(&me->client, &localSend);
}

void 
recall(MqttClient *const me, Event *pe)
{
    recallEvent(me, &deferQueue);
}

void 
defer(MqttClient *const me, Event *pe)
{
    deferEvent(&deferQueue, pe);
}

/* ............................. Entry actions ............................. */
void 
recvAll(MqttClient *const me, Event *pe)
{
	(void)pe;

    ConnectionTopic_publish(&evRecvObj, me);
}

void 
sendAll(MqttClient *const me, Event *pe)
{
	(void)pe;

    evSendObj.size = localSend.msg->size;
    memcpy(evSendObj.buf, localSend.msg->start, localSend.msg->size);
    ConnectionTopic_publish(&evSendObj, me);
}

/* ............................. Exit actions .............................. */
/* ................................ Guards ................................. */
rbool_t 
isRecvBufFull(MqttClient *const me, Event *pe)
{
	(void)me;
	(void)pe;

    return mqtt_isRecvBufFull(&localRecv);
}

rbool_t 
isInitOk(MqttClient *const me, Event *pe)
{
	(void)me;
	(void)pe;

    return mqtt_isInitOk(&localSend);
}

rbool_t 
isThereMsg(MqttClient *const me, Event *pe)
{
	(void)me;
	(void)pe;

    return mqtt_isThereMsg(&localSend);
}

rbool_t 
isNotResend(MqttClient *const me, Event *pe)
{
	(void)me;
	(void)pe;

    return localSend.resend == 0;
}

rbool_t 
isSetMsgStateOk(MqttClient *const me, Event *pe)
{
	(void)me;
	(void)pe;

    return mqtt_isSetMsgStateResult(&localSend);
}

rbool_t 
isUnpackError(MqttClient *const me, Event *pe)
{
	(void)me;
	(void)pe;

    return mqtt_isUnpackError(&localRecv);
}

rbool_t 
isConsumed(MqttClient *const me, Event *pe)
{
	(void)me;
	(void)pe;

    return mqtt_isConsumed(&localRecv);
}

rbool_t 
isNotError(MqttClient *const me, Event *pe)
{
	(void)me;
	(void)pe;

    return mqtt_isNotError(&localRecv);
}

/* ------------------------------ File footer ------------------------------ */

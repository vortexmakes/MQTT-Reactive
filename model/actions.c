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
/* ---------------------------- Local data types --------------------------- */
/* ---------------------------- Global variables --------------------------- */
/* ---------------------------- Local variables ---------------------------- */
static RKH_QUEUE_T deferQueue;
static RKH_EVT_T *deferQueueStorage[MAX_SIZEOF_DEFERQUEUE];

/* ----------------------- Local function prototypes ----------------------- */
/* ---------------------------- Local functions ---------------------------- */
static void
connack_response_callback(enum MQTTConnackReturnCode return_code)
{
    MQTTProt *me;
    RKH_EVT_T *evt;

    me = RKH_DOWNCAST(MQTTProt, mqttProt);
    if (return_code == MQTT_CONNACK_ACCEPTED)
    {
        evt = RKH_UPCAST(RKH_EVT_T, &evConnAcceptedObj);
    }
    else
    {
        evConnRefusedObj.code = return_code;
        evt = RKH_UPCAST(RKH_EVT_T, &evConnRefusedObj);
    }
    RKH_SMA_POST_FIFO(RKH_UPCAST(RKH_SMA_T, me), evt, me);
}

/* ---------------------------- Global functions --------------------------- */
/* ............................ Effect actions ............................. */
void
initialize(...)
{
    rkh_queue_init(&deferQueue, 
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
    rui16_t pubTime;

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
initRecvAll(SyncRegion *const me, RKH_EVT_T *pe)
{
	(void)pe;

    mqtt_isReconnect(me->client);
    mqtt_initRecvAll();
}

void 
parseRecv(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;
    ReceivedEvt *evt;

    realMe = me->itsMQTTProt;
    evt = RKH_DOWNCAST(ReceivedEvt, pe);

    memcpy(realMe->client.recv_buffer.curr, evt->buf, evt->size);
    localRecv.rv = evt->size;
    mqtt_parseRecv(&realMe->client, &localRecv);
}

void 
recvFail(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

	(void)pe;

    realMe = me->itsMQTTProt;
    localRecv.rv = MQTT_ERROR_SOCKET_ERROR;
    mqtt_recvFail(&realMe->client, &localRecv); /* an error occurred */
}

void 
parseError(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

	(void)pe;

    realMe = me->itsMQTTProt;
    mqtt_parseError(&realMe->client, &localRecv);
}

void 
noConsumed(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

	(void)pe;

    realMe = me->itsMQTTProt;
    mqtt_noConsumed(&realMe->client, &localRecv);
}

void 
handleRecvMsg(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

	(void)pe;

    realMe = me->itsMQTTProt;
    mqtt_handleRecvMsg(&realMe->client, &localRecv);
}

void 
recvMsgError(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

	(void)pe;

    realMe = me->itsMQTTProt;
    mqtt_recvMsgError(&realMe->client, &localRecv);
}

void 
cleanBuf(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

	(void)pe;

    realMe = me->itsMQTTProt;
    mqtt_cleanBuf(&realMe->client, &localRecv);
}

void 
initSendAll(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

	(void)pe;

    realMe = me->itsMQTTProt;
    mqtt_initSendAll(&realMe->client, &localSend);
}

void 
initSendAll(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

	(void)pe;

    realMe = me->itsMQTTProt;
    mqtt_initSendAll(&realMe->client, &localSend);
}

void 
endSendAll(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

	(void)pe;

    realMe = me->itsMQTTProt;
    mqtt_endSendAll(&realMe->client);
}

void 
sendOneMsg(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

	(void)pe;

    realMe = me->itsMQTTProt;
    mqtt_sendOneMsg(&realMe->client, &localSend);
}

void 
nextSend(SyncRegion *const me, RKH_EVT_T *pe)
{
	(void)me;
	(void)pe;

    mqtt_nextSend(&localSend);
}

void 
sendMsgFail(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

	(void)pe;

    realMe = me->itsMQTTProt;
    localSend.tmp = MQTT_ERROR_SOCKET_ERROR;
    mqtt_sendMsgFail(&realMe->client, &localSend);
}

void 
setMsgState(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

	(void)pe;

    realMe = me->itsMQTTProt;
    mqtt_setMsgState(&realMe->client, &localSend);
}

void 
recall(SyncRegion *const me, RKH_EVT_T *pe)
{
    rkh_sma_recall(me, &deferQueue);
}

void 
defer(SyncRegion *const me, RKH_EVT_T *pe)
{
    rkh_sma_defer(&deferQueue, pe);
}

/* ............................. Entry actions ............................. */
void 
recvAll(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

	(void)pe;

    realMe = me->itsMQTTProt;
    ConnectionTopic_publish(&evRecvObj, realMe);
}

void 
sendAll(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

	(void)pe;

    realMe = me->itsMQTTProt;
    evSendObj.size = localSend.msg->size;
    memcpy(evSendObj.buf, localSend.msg->start, localSend.msg->size);
    ConnectionTopic_publish(&evSendObj, realMe);
}

/* ............................. Exit actions .............................. */
/* ................................ Guards ................................. */
rbool_t 
isRecvBufFull(const RKH_SM_T *me, RKH_EVT_T *pe)
{
	(void)me;
	(void)pe;

    return mqtt_isRecvBufFull(&localRecv);
}

rbool_t 
isInitOk(const RKH_SM_T *me, RKH_EVT_T *pe)
{
	(void)me;
	(void)pe;

    return mqtt_isInitOk(&localSend);
}

rbool_t 
isThereMsg(const RKH_SM_T *me, RKH_EVT_T *pe)
{
	(void)me;
	(void)pe;

    return mqtt_isThereMsg(&localSend);
}

rbool_t 
isNotResend(const RKH_SM_T *me, RKH_EVT_T *pe)
{
	(void)me;
	(void)pe;

    return localSend.resend == 0;
}

rbool_t 
isSetMsgStateOk(const RKH_SM_T *me, RKH_EVT_T *pe)
{
	(void)me;
	(void)pe;

    return mqtt_isSetMsgStateResult(&localSend);
}

rbool_t 
isUnpackError(const RKH_SM_T *me, RKH_EVT_T *pe)
{
	(void)me;
	(void)pe;

    return mqtt_isUnpackError(&localRecv);
}

rbool_t 
isConsumed(const RKH_SM_T *me, RKH_EVT_T *pe)
{
	(void)me;
	(void)pe;

    return mqtt_isConsumed(&localRecv);
}

rbool_t 
isNotError(const RKH_SM_T *me, RKH_EVT_T *pe)
{
	(void)me;
	(void)pe;

    return mqtt_isNotError(&localRecv);
}

/* ------------------------------ File footer ------------------------------ */

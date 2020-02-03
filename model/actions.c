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
reconnectSync(SyncRegion *const me, RKH_EVT_T *pe)
{
    MQTTProt *realMe;

	(void)pe;

    realMe = me->itsMQTTProt;
    RKH_SMA_POST_LIFO(RKH_UPCAST(RKH_SMA_T, realMe), 
                      RKH_UPCAST(RKH_EVT_T, &evDeactivateObj), 
                      realMe);
}

/* ............................. Entry actions ............................. */
/* ............................. Exit actions .............................. */
/* ................................ Guards ................................. */
rbool_t 
isReconnect(const RKH_SM_T *me, RKH_EVT_T *pe)
{
    SyncRegion *realMe;

	(void)pe;

    realMe = RKH_DOWNCAST(SyncRegion, me);
    return mqtt_isReconnect(&realMe->itsMQTTProt->client);
}

/* ------------------------------ File footer ------------------------------ */

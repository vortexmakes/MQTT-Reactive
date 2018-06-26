/**
 *  \file       rkhevt.h
 *  \brief      Defines event data type and other related macros.
 */

/* -------------------------- Development history -------------------------- */
/*
 *  2018.06.09  LeFr  v1.0.00  Initial version
 */

/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci  lf@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* --------------------------------- Module -------------------------------- */
#ifndef __MQTT_SYNC_H__
#define __MQTT_SYNC_H__

/* ----------------------------- Include files ----------------------------- */
#include "mqtt.h"

/* ---------------------- External C language linkage ---------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------- Macros -------------------------------- */
/* -------------------------------- Constants ------------------------------ */
/* ------------------------------- Data types ------------------------------ */
typedef struct LocalSendAll LocalSendAll;
struct LocalSendAll
{
    uint8_t inspected;
    int len;
    int inflight_qos2;
    int i;
    int resend;
    ssize_t tmp;
    struct mqtt_queued_message *msg;
    int initResult;
    int setMsgStateResult;
};

typedef struct LocalRecvAll LocalRecvAll;
struct LocalRecvAll
{
    ssize_t rv;
    ssize_t consumed;
    struct mqtt_response response;
    struct mqtt_queued_message *msg;
    int handleRecvMsgResult;
    int noConsumedResult;
};

/* -------------------------- External variables --------------------------- */
/* -------------------------- Function prototypes -------------------------- */
void mqtt_initSendAll(struct mqtt_client *client, LocalSendAll *local);
int mqtt_isThereMsg(LocalSendAll *local);
void mqtt_sendOneMsg(struct mqtt_client *client, LocalSendAll *local);
void mqtt_sendMsgFail(struct mqtt_client *client, LocalSendAll *local);
void mqtt_setMsgState(struct mqtt_client *client, LocalSendAll *local);
int mqtt_isInitOk(LocalSendAll *local);
int mqtt_isSetMsgStateResult(LocalSendAll *local);
void mqtt_nextSend(LocalSendAll *local);
int mqtt_endSendAll(struct mqtt_client *client);

void mqtt_initRecvAll(void);
void mqtt_recvAll(struct mqtt_client *client, LocalRecvAll *local);
void mqtt_recvFail(struct mqtt_client *client, LocalRecvAll *local);
void mqtt_parseRecv(struct mqtt_client *client, LocalRecvAll *local);
int mqtt_isConsumed(LocalRecvAll *local);
int mqtt_isUnpackError(LocalRecvAll *local);
void mqtt_parseError(struct mqtt_client *client, LocalRecvAll *local);
int mqtt_noConsumed(struct mqtt_client *client, LocalRecvAll *local);
int mqtt_isNotError(LocalRecvAll *local);
void mqtt_handleRecvMsg(struct mqtt_client *client, LocalRecvAll *local);
void mqtt_cleanBuf(struct mqtt_client *client, LocalRecvAll *local);
void mqtt_recvMsgError(struct mqtt_client *client, LocalRecvAll *local);
int mqtt_isRecvBufFull(LocalRecvAll *local);
int mqtt_isReconnect(struct mqtt_client *client);

/* -------------------- External C language linkage end -------------------- */
#ifdef __cplusplus
}
#endif

/* ------------------------------ Module end ------------------------------- */
#endif

/* ------------------------------ End of file ------------------------------ */

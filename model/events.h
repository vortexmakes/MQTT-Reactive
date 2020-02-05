/**
 *  \file       events.h
 *  \brief      Event data types specifications.
 */

/* -------------------------- Development history -------------------------- */
/* -------------------------------- Authors -------------------------------- */
/*
 *  LeFr  Leandro Francucci lf@vortexmakes.com
 */

/* --------------------------------- Notes --------------------------------- */
/* --------------------------------- Module -------------------------------- */
#ifndef __EVENTS_H__
#define __EVENTS_H__

/* ----------------------------- Include files ----------------------------- */
/* ---------------------- External C language linkage ---------------------- */
#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------- Macros -------------------------------- */
/* -------------------------------- Constants ------------------------------ */
/* ------------------------------- Data types ------------------------------ */
typedef struct ConnRefusedEvt ConnRefusedEvt;
struct ConnRefusedEvt
{
    Event evt;
    enum MQTTConnackReturnCode code;
};

typedef struct SendEvt SendEvt;
struct SendEvt
{
    Event evt;
    unsigned char buf[SEND_BUFF_SIZE];
    unsigned int size;
};

typedef struct ReceivedEvt ReceivedEvt;
struct ReceivedEvt
{
    Event evt;
    unsigned char buf[RECV_BUFF_SIZE];
    unsigned int size;
};

/* -------------------------- External variables --------------------------- */
/* -------------------------- Function prototypes -------------------------- */
/* -------------------- External C language linkage end -------------------- */
#ifdef __cplusplus
}
#endif

/* ------------------------------ Module end ------------------------------- */
#endif

/* ------------------------------ End of file ------------------------------ */

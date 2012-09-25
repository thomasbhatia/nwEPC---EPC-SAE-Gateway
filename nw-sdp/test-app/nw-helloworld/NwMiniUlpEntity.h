/*----------------------------------------------------------------------------*
 *                                                                            *
 *            M I N I M A L I S T I C     U L P     E N T I T Y               *
 *                                                                            *
 *                    Copyright (C) 2010 Amit Chawre.                         *
 *                                                                            *
 *----------------------------------------------------------------------------*/

/** 
 * @file NwMiniUlpEntity.h
 * @brief This file contains example of a minimalistic ULP entity.
*/

#include <stdio.h>
#include <assert.h>
#include "NwEvt.h"
#include "NwLog.h"

#ifndef __NW_MINI_ULP_H__ 
#define __NW_MINI_ULP_H__ 

typedef struct
{
  int                           greSd;
  NwU16T                        seqNum;
  NwU8T                         restartCounter;
  NwU8T                         localIpStr[16];
  NwU8T                         peerIpStr[16];
  NwU32T                        localPort[1025];
  NwEventT                      greEv;
  NwEventT                      ev[1025];
  NwSdpHandleT                  hSdp;
  NwSdpSessionHandleT           hGtpv1uConn;
} NwMiniUlpEntityT;

#ifdef __cplusplus
extern "C" {
#endif

NwSdpRcT
nwMiniUlpInit(NwMiniUlpEntityT* thiz, NwSdpHandleT hSdp);

NwSdpRcT
nwMiniUlpDestroy(NwMiniUlpEntityT* thiz);

NwSdpRcT
nwMiniUlpCreateConn(NwMiniUlpEntityT* thiz, char* localIpStr, NwU16T localPort, char* peerIpStr);

NwSdpRcT
nwMiniUlpDestroyConn(NwMiniUlpEntityT* thiz);

NwSdpRcT
nwMiniUlpSendMsg(NwMiniUlpEntityT* thiz);

NwSdpRcT
nwMiniUlpTpduSend(NwMiniUlpEntityT* thiz, NwU8T* tpduBuf, NwU32T tpduLen , NwU16T fromPort);

NwSdpRcT 
nwMiniUlpProcessStackReqCallback (NwSdpUlpHandleT hUlp, 
                       NwSdpUlpApiT *pUlpApi);

#ifdef __cplusplus
}
#endif

#endif

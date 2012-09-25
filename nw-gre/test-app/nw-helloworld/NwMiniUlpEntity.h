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
#ifndef __NW_MINI_ULP_H__ 
#define __NW_MINI_ULP_H__ 

typedef struct
{
  int                           hSocket;
  NwU16T                        seqNum;
  NwU8T                         restartCounter;
  NwU8T                         localIpStr[16];
  NwU8T                         peerIpStr[16];
  NwU32T                        localPort[1025];
  NwEventT                      ev[1025];
  NwGreStackHandleT             hGreStack;
  NwGreStackSessionHandleT      hGreConn;
} NwMiniUlpEntityT;

#ifdef __cplusplus
extern "C" {
#endif

NwRcT
nwMiniUlpInit(NwMiniUlpEntityT* thiz, NwGreStackHandleT hGreStack);

NwRcT
nwMiniUlpDestroy(NwMiniUlpEntityT* thiz);

NwRcT
nwMiniUlpCreateConn(NwMiniUlpEntityT* thiz, char* localIpStr, NwU16T localPort, char* peerIpStr);

NwRcT
nwMiniUlpDestroyConn(NwMiniUlpEntityT* thiz);

NwRcT
nwMiniUlpSendMsg(NwMiniUlpEntityT* thiz);

NwRcT
nwMiniUlpTpduSend(NwMiniUlpEntityT* thiz, NwU8T* tpduBuf, NwU32T tpduLen , NwU16T fromPort);

NwRcT 
nwMiniUlpProcessStackReqCallback (NwGreUlpHandleT hUlp, 
                       NwGreUlpApiT *pUlpApi);

#ifdef __cplusplus
}
#endif

#endif

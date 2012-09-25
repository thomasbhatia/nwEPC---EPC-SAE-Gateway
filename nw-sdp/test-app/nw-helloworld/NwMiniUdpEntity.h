/*----------------------------------------------------------------------------*
 *                                                                            *
 *            M I N I M A L I S T I C     U D P     E N T I T Y               *
 *                                                                            *
 *                    Copyright (C) 2010 Amit Chawre.                         *
 *                                                                            *
 *----------------------------------------------------------------------------*/


/** 
 * @file NwMiniUdpEntity.h
 * @brief This file contains example of a minimalistic ULP entity.
*/

#include <stdio.h>
#include <assert.h>
#include "NwEvt.h"
#include "NwLog.h"

#ifndef NW_ASSERT
#define NW_ASSERT assert
#endif 

#ifndef __NW_MINI_UDP_ENTITY_H__
#define __NW_MINI_UDP_ENTITY_H__

typedef struct
{
  NwU32T        hSocket;
  NwEventT      ev;
  NwSdpHandleT hSdp;
} NwMiniUdpEntityT;

#ifdef __cplusplus
extern "C" {
#endif

NwSdpRcT nwMiniUdpInit(NwMiniUdpEntityT* thiz, NwSdpHandleT hSdp, NwU8T* ipAddr);

NwSdpRcT nwMiniUdpDestroy(NwMiniUdpEntityT* thiz);

NwSdpRcT
nwMiniUdpCreateGreInterface(NwMiniUdpEntityT* thiz, char* localIpStr, NwU32T* phGreInterface);

NwSdpRcT nwMiniUdpGreDataReq(NwU32T hGreInterface,
    NwU8T* dataBuf,
    NwU32T dataSize,
    NwU32T peerAddr,
    NwU32T peerPort);

NwSdpRcT nwMiniUdpDataReq(NwSdpUdpHandleT udpHandle,
    NwU8T* dataBuf,
    NwU32T dataSize,
    NwU32T peerAddr,
    NwU32T peerPort);

#ifdef __cplusplus
}
#endif

#endif

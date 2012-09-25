/*----------------------------------------------------------------------------*
 *                                                                            *
 *            M I N I M A L I S T I C     U D P     E N T I T Y               *
 *                                                                            *
 *                    Copyright (C) 2010 Amit Chawre.                         *
 *                                                                            *
 *----------------------------------------------------------------------------*/


/** 
 * @file NwMiniLlpEntity.h
 * @brief This file contains example of a minimalistic ULP entity.
*/

#include <stdio.h>
#include <assert.h>
#include "NwEvt.h"

#ifndef NW_ASSERT
#define NW_ASSERT assert
#endif 

#ifndef __NW_MINI_UDP_ENTITY_H__
#define __NW_MINI_UDP_ENTITY_H__

typedef struct
{
  NwU32T        hSocket;
  NwEventT      ev;
  NwIpv4StackHandleT hIpv4Stack;
} NwMiniLlpEntityT;

#ifdef __cplusplus
extern "C" {
#endif

NwIpv4RcT nwMiniUdpInit(NwMiniLlpEntityT* thiz, NwIpv4StackHandleT hIpv4Stack, NwU8T* ipAddr);

NwIpv4RcT nwMiniUdpDestroy(NwMiniLlpEntityT* thiz);

NwIpv4RcT nwMiniUdpDataReq(NwIpv4LlpHandleT llpHandle,
    NwU8T* dataBuf,
    NwU32T dataSize,
    NwU32T peerAddr,
    NwU32T peerPort);

#ifdef __cplusplus
}
#endif

#endif

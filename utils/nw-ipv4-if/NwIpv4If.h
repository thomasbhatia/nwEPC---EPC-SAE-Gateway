/*----------------------------------------------------------------------------*
 *                                                                            *
 * Copyright (c) 2010-2011 Amit Chawre                                        *
 * All rights reserved.                                                       *
 *                                                                            *
 * Redistribution and use in source and binary forms, with or without         *
 * modification, are permitted provided that the following conditions         *
 * are met:                                                                   *
 *                                                                            *
 * 1. Redistributions of source code must retain the above copyright          *
 *    notice, this list of conditions and the following disclaimer.           *
 * 2. Redistributions in binary form must reproduce the above copyright       *
 *    notice, this list of conditions and the following disclaimer in the     *
 *    documentation and/or other materials provided with the distribution.    *
 * 3. The name of the author may not be used to endorse or promote products   *
 *    derived from this software without specific prior written permission.   *
 *                                                                            *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR       *
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES  *
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.    *
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,           *
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT   *
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  *
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY      *
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT        *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF   *
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.          *
 *----------------------------------------------------------------------------*/

/** 
 * @file NwIpv4If.h
 * @brief This files defines IP interface entity.
 */

#ifndef __NW_IP_IF_H__
#define __NW_IP_IF_H__

#include "NwEvt.h"
#include "NwTypes.h"
#include "NwError.h"
#include "NwSdp.h"

/*---------------------------------------------------------------------------
 *                         U D P     E N T I T Y 
 *--------------------------------------------------------------------------*/

#define MAX_IP_PAYLOAD_LEN                                     (4096)

typedef struct
{
  NwU32T        hRecvSocketIpv4;
  NwU32T        hRecvSocketArp;
  NwU32T        hSendSocket;
  NwU32T        ifindex;
  NwU8T         hwAddr[6];
  NwU32T        ipAddr;
  NwEventT      evIpv4;
  NwEventT      evArp;
  NwSdpHandleT  hSdp;
} NwIpv4IfT;

#ifdef __cplusplus
extern "C" {
#endif

NwRcT nwIpv4IfInitialize(NwIpv4IfT* thiz, NwU8T* device, NwSdpHandleT hSdp, NwU8T* pHwAddr);

NwRcT nwIpv4IfDestroy(NwIpv4IfT* thiz);

NwRcT nwIpv4IfGetSelectionObjectIpv4(NwIpv4IfT* thiz, NwU32T *pSelObj);

NwRcT nwIpv4IfGetSelectionObjectArp(NwIpv4IfT* thiz, NwU32T *pSelObj);

void NW_EVT_CALLBACK(nwIpv4IfDataIndicationCallback);

void NW_EVT_CALLBACK(nwIpv4IfArpDataIndicationCallback);

NwRcT 
nwIpv4IfIpv4DataReq(NwSdpHandleT hSdp,
             NwU8T* dataBuf,
             NwU32T dataSize);

NwRcT nwIpv4IfArpDataReq(NwSdpHandleT       hThiz,
                     NwU16T             opCode,
                     NwU8T              *pTargetMac,
                     NwU8T              *pTargetIpAddr,
                     NwU8T              *pSenderIpAddr);


#ifdef __cplusplus
}
#endif


#endif

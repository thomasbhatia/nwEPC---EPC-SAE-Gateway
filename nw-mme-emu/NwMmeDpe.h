/*----------------------------------------------------------------------------*
 *                                                                            *
 *                                n w - m m e                                 * 
 *    L T E / S A E    M O B I L I T Y   M A N A G E M E N T   E N T I T Y    *
 *                                                                            *
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
 * @file nwMmeDpe.h
 */

#ifndef __NW_MMEDPE_H__
#define __NW_MMEDPE_H__

#include <stdio.h>
#include <assert.h>

#include "NwEvt.h"
#include "NwLog.h"
#include "NwTypes.h"
#include "NwUtils.h"
#include "NwSdp.h"
#include "NwIpv4If.h"
#include "NwGtpv1uIf.h"

typedef struct {
  NwSdpHandleT  hSdp;
  NwIpv4IfT     ipv4If;
  NwGtpv1uIfT   gtpuIf;
  NwEventT      evGtpuIf;
  NwEventT      evIpv4;
  NwSdpServiceHandleT hGtpu;
  NwSdpServiceHandleT hIpv4;
} NwMmeDpeT;

#ifdef __cplusplus
extern "C" {
#endif

extern NwMmeDpeT*
nwMmeDpeInitialize();

extern NwRcT
nwMmeDpeDestroy(NwMmeDpeT* thiz);

/**
 * Create Gtpu to Ipv4 flow with Soft Data Plane
 */

extern NwRcT
nwMmeDpeCreateGtpuIpv4Flow(NwMmeDpeT*   thiz, 
                         NwU32T         hSession,
                         NwU32T         teidIngress,
                         NwU32T         *pTeidIngress,
                         NwU32T         *pIpv4Ingress,
                         NwU32T         *phBearer);

/**
 * Create Ipv4 to Gtpu flow with Soft Data Plane
 */

extern NwRcT
nwMmeDpeCreateIpv4GtpuFlow(NwMmeDpeT*   thiz, 
                         NwU32T         hSession,
                         NwU32T         teidEgress,
                         NwU32T         ipv4Egress,
                         NwU32T         ipv4Ingress,
                         NwU32T         *phBearer);

/**
 * Create Gtpu to Gtpu flow with Soft Data Plane
 */

extern NwRcT
nwMmeDpeCreateGtpuGtpuFlow(NwMmeDpeT*   thiz, 
                         NwU32T         hSession,
                         NwU32T         teidEgress,
                         NwU32T         ipv4Egress,
                         NwU32T         *teidIngress,
                         NwU32T         *ipv4Ingress,
                         NwU32T         *phBearer);

/**
 * Destroy a flow with Soft Data Plane
 */

extern NwRcT
nwMmeDpeDestroyFlow(NwMmeDpeT*   thiz, 
                      NwU32T         hBearer);
extern NwRcT
nwMmeDpeCreateGtpuService( NwMmeDpeT* thiz, NwU32T ipv4Addr );

extern NwRcT
nwMmeDpeCreateIpv4Service( NwMmeDpeT* thiz, NwU8T* nwIfName);

#ifdef __cplusplus
}
#endif

#endif

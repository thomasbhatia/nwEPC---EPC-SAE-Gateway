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
 * @file NwMmeUlp.h
 */

#ifndef __NW_MME_ULP_H__
#define __NW_MME_ULP_H__

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "NwEvt.h"
#include "NwLog.h"
#include "NwUtils.h"
#include "NwMmeLog.h"
#include "NwLogMgr.h"
#include "NwGtpv2c.h"
#include "NwMmeUe.h"
#include "NwMmeDpe.h"
#include "NwUdp.h"


#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------
 *                    M M E      U L P     E N T I T Y 
 *--------------------------------------------------------------------------*/
typedef struct
{
  NwU32T                        rps;            /**< Registrations per second */
  NwU32T                        maxUeSessions;
  NwU32T                        sessionTimeout;
  NwU32T                        sessionCount;
  NwU32T                        hStatsTimer;
  NwU32T                        hRpsTimer;
  NwU32T                        mmeIpAddr;
  NwU32T                        sgwIpAddr;
  NwU32T                        pgwIpAddr;
  NwGtpv2cStackHandleT          hGtpcStack;
  NwMmeDpeT*                    pDpe;
  NwMmeUeT*                     pMsCurrent;
  NwMmeUeT*                     pMsListHead;
  NwMmeUeT*                     pMsListTail;
  NwMmeUeT*                     pMsListDeregisteredHead;
  NwMmeUeT*                     pMsListDeregisteredTail;
} NwMmeUlpT;

NwRcT
nwMmeUlpInit(NwMmeUlpT*  thiz, 
             NwU32T             maxUeSessions,
             NwU32T             sessionTimeout,
             NwU32T             mmeIpAddr,
             NwU32T             sgwIpAddr,
             NwU32T             pgwIpAddr,
             NwU32T             rps,
             NwMmeDpeT          *pDpe,
             NwGtpv2cStackHandleT hGtpcStack);


NwRcT
nwMmeUlpDestroy(NwMmeUlpT*  thiz);

NwRcT
nwMmeUlpStartNetworkEntry(NwMmeUlpT* thiz);

NwRcT 
nwMmeUlpStackReqCallback (NwGtpv2cUlpHandleT hUlp, 
                            NwGtpv2cUlpApiT *pUlpApi);

NwRcT
nwMmeUlpGetControlPlaneIpv4Addr(NwMmeUlpT* thiz, NwU32T *pMmeControlPlaneIpv4Addr);

#ifdef __cplusplus
}
#endif

#endif

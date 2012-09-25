/*----------------------------------------------------------------------------*
 *                                                                            *
 *                                n w - e p c                                 * 
 *       L T E / S A E        S E R V I N G / P D N       G A T E W A Y       *
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
 * @file NwSaeGwUlp.h
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "tree.h"
#include "queue.h"
#include "NwEvt.h"
#include "NwLog.h"
#include "NwGtpv2cIf.h"
#include "NwUtils.h"
#include "NwIpv4PoolMgr.h"
#include "NwSaeGwLog.h"
#include "NwLogMgr.h"
#include "NwGtpv2c.h"
#include "NwSaeGwDpe.h"
//#include "NwSaeGwUe.h"
#include "NwSaeGwUeFsm.h"

#ifndef __NW_SAE_GW_ULP_H__
#define __NW_SAE_GW_ULP_H__


#ifdef __cplusplus
extern "C" {
#endif

#define NW_SAE_GW_TYPE_SGW                      (1)
#define NW_SAE_GW_TYPE_PGW                      (2)

typedef struct 
{
  NwU32T                        maxUeSessions;
  NwU32T                        s11cIpv4Addr;
  NwU32T                        s5cIpv4AddrSgw;
  NwU32T                        s4cIpv4AddrSgw;
  NwU32T                        s5cIpv4AddrPgw;
  NwU8T                         apn[1024];
  NwU32T                        ippoolSubnet;
  NwU32T                        ippoolMask;
  NwSaeGwDpeT                   *pDpe;
} NwSaeGwUlpConfigT;

typedef struct 
{
  NwGtpv2cStackHandleT          hGtpv2cStack;
  NwGtpv2cIfT                   udpIf;
  NwEventT                      ev;
  NwU32T                        ipv4Addr;
} NwSaeGwGtpv2cSapT; 

typedef struct NwSaeGwUlp
{
  NwU32T                        saeGwType;
  NwU32T                        maxUeSessions;
  NwU8T                         apn[1024];
  NwIpv4PoolMgrHandleT          hIpv4Pool;
  NwU32T                        ippoolSubnet;
  NwU32T                        ippoolMask;
  NwU32T                        s11cIpv4Addr;
  NwSaeGwUeFsmT                 *pUeFsm;

  struct
  {
    NwSaeGwGtpv2cSapT           s11c;
    NwSaeGwGtpv2cSapT           s5c;
    NwSaeGwGtpv2cSapT           s4c;
  } sgw;

  struct
  {
    NwSaeGwGtpv2cSapT           s5c;
  } pgw;

  NwU32T                        ipv4AddrPool;
  NwSaeGwDpeT                   *pDpe;
  TAILQ_ENTRY(NwSaeGwUlp)       collocatedPgwListNode;
  TAILQ_HEAD(NwSaeGwUlpCollatedPgwListT, NwSaeGwUlp) collocatedPgwList;
  RB_HEAD(NwUeSgwSessionRbtT, NwSaeGwUe) ueSgwSessionRbt;
  RB_HEAD(NwUePgwSessionRbtT, NwSaeGwUe) uePgwSessionRbt;
} NwSaeGwUlpT;

NwSaeGwUlpT*
nwSaeGwUlpNew();

NwRcT
nwSaeGwUlpDelete(NwSaeGwUlpT*  thiz);

NwRcT
nwSaeGwUlpInitialize(NwSaeGwUlpT*     thiz, 
               NwU32T           type,
               NwSaeGwUlpConfigT *cfg);

NwRcT
nwSaeGwUlpDestroy(NwSaeGwUlpT*  thiz);

NwRcT
nwSaeGwUlpRegisterCollocatedPgw(NwSaeGwUlpT* thiz, NwSaeGwUlpT* pCollocatedPgw);

NwRcT
nwSaeGwUlpDeregisterCollocatedPgw(NwSaeGwUlpT* thiz, NwSaeGwUlpT* pCollocatedPgw);

NwRcT
nwSaeGwUlpRegisterSgwUeSession(NwU32T hSgw, NwSaeGwUeT *pUe, NwU32T pgwIpv4Addr, NwU32T *hPgw);

NwRcT
nwSaeGwUlpRegisterPgwUeSession(NwU32T hPgw, NwSaeGwUeT *pUe);

NwRcT
nwSaeGwUlpSgwDeregisterUeSession(NwU32T hSaeGw, NwSaeGwUeT *pUe);

NwRcT
nwSaeGwUlpPgwDeregisterUeSession(NwU32T hSaeGw, NwSaeGwUeT *pUe);

NwRcT
nwSaeGwUlpAllocateTeidOrGreKeys(NwU32T hSaeGw, NwSaeGwUeT *pUe, NwU8T ebi);

NwRcT
nwSaeGwUlpInstallUplinkEpsBearer(NwU32T hSaeGw, NwSaeGwUeT *pUe, NwU8T ebi);

NwRcT
nwSaeGwUlpRemoveUplinkEpsBearer(NwU32T hSaeGw, NwSaeGwUeT *pUe, NwU8T ebi);

NwRcT
nwSaeGwUlpInstallDownlinkEpsBearer(NwU32T hSaeGw, NwSaeGwUeT *pUe, NwU8T ebi);

NwRcT
nwSaeGwUlpModifyDownlinkEpsBearer(NwU32T hSaeGw, NwSaeGwUeT *pUe, NwU8T ebi);

NwRcT
nwSaeGwUlpRemoveDownlinkEpsBearer(NwU32T hSaeGw, NwSaeGwUeT *pUe, NwU8T ebi);

#ifdef __cplusplus
}
#endif


#endif

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
 * @file NwMmeUe.h
*/

#ifndef __NW_MME_UE__
#define __NW_MME_UE__

#include <stdio.h>
#include <assert.h>

#include "NwTmrMgr.h"
#include "NwLog.h"
#include "NwTypes.h"
#include "NwGtpv2c.h"
#include "NwMmeDpe.h"
#include "NwGtpv2cMsgParser.h"

typedef enum
{
  NW_MME_UE_STATE_INIT = 0,
  NW_MME_UE_STATE_CREATE_SESSION_REQUEST_SENT,
  NW_MME_UE_STATE_MODIFY_BEARER_REQUEST_SENT,
  NW_MME_UE_STATE_DELETE_SESSION_REQUEST_SENT,
  NW_MME_UE_STATE_SESSION_CREATED,
  NW_MME_UE_STATE_END
} NwUeStateT;

typedef enum
{
  NW_MME_UE_EVENT_GTPC_MSG_INDICATION = 0,
  NW_MME_UE_EVENT_GTPC_INITIAL_REQ_MSG_INDICATION,
  NW_MME_UE_EVENT_GTPC_TRIGGERED_RSP_MSG_INDICATION,
  NW_MME_UE_EVENT_GTPC_TRIGGERED_REQ_MSG_INDICATION,
  NW_MME_UE_EVENT_NETWORK_ENTRY_START,
  NW_MME_UE_EVENT_NACK,
  NW_MME_UE_EVENT_SESSION_TIMEOUT,
  NW_MME_UE_EVENT_END
} NwMmeUeEventT;

/**
 * Fully Qualified Tunnel Endpoint Identifier aka FTEID
 */
typedef struct
{
  NwBoolT isIpv4;
  NwBoolT isIpv6;
  NwU8T   ifType;
  NwU32T  teidOrGreKey;
  NwU32T  ipv4Addr;
  NwU8T   ipv6Addr[16];
} NwMmeFteidT;

/**
 * EPS Bearer  
 */
typedef  struct 
{
  NwU8T ebi;

  struct {
    NwMmeFteidT               fteidEnodeB;
    NwMmeFteidT               fteidSgw;
  } s1u;

  struct {
    NwMmeFteidT               fteidPgw;
    NwMmeFteidT               fteidSgw;
  } s5s8u;

} NwMmeEpsBearerT;

typedef NwPtrT NwDpeBearerHandleT;

typedef struct NwMmeUe
{
  NwU8T                         imsi[8];
  NwU8T                         msIsdn[8];
  NwU8T                         mei[8];
  NwU8T                         servingNetwork[3];
  NwU32T                        hMmeUlp;
  NwGtpv2cStackHandleT          hGtpcStack;
  NwGtpv2cTunnelHandleT         hGtpv2cTunnel;
  NwU32T                        pdnIpv4Addr;
  NwU32T                        mmeIpv4Addr;
  NwU32T                        sgwIpv4Addr;
  NwU32T                        pgwIpv4Addr;
  NwUeStateT                    state;
  NwMmeFteidT                   fteidControlPeer;
  NwMmeDpeT                     *pDpe;
  NwU32T                        sessionTimeout;
  NwTimerHandleT                hSessionTimer;
  struct NwMmeUe                *next;
  struct NwMmeUe                *registeredNext;
  struct NwMmeUe                *deregisteredNext;
#define NW_SAE_GW_MAX_EPS_BEARERS               (16)
  struct {
    NwBoolT isValid;

    struct {
      NwMmeFteidT               fteidEnodeB;
      NwMmeFteidT               fteidSgw;
      NwDpeBearerHandleT        hUplink;
      NwDpeBearerHandleT        hDownlink;
    } s1uTunnel;

    struct {
      NwMmeFteidT               fteidPgw;
    } s5s8Tunnel;

  } epsBearer[NW_SAE_GW_MAX_EPS_BEARERS];

} NwMmeUeT;

typedef struct
{
  NwMmeUeEventT event;
  void*  arg;
} NwMmeUeEventInfoT;

#ifdef __cplusplus
extern "C" {
#endif

NwMmeUeT*
nwMmeUeNew();

NwRcT
nwMmeUeDelete(NwMmeUeT*);

NwRcT
nwMmeUeSetImsi(NwMmeUeT* thiz, NwU8T* imsi);

NwRcT
nwMmeUeBuildSgwCreateSessionResponseParser(NwMmeUeT* thiz, NwGtpv2cMsgParserT** ppMsgParser);

NwRcT
nwMmeUeFsmRun(NwMmeUeT* thiz, NwMmeUeEventInfoT* pEv);

#ifdef __cplusplus
}
#endif


#endif

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
 * @file NwSaeGwUe.h
*/

#include <stdio.h>
#include <assert.h>

#include "tree.h"
#include "NwLog.h"
#include "NwTypes.h"
#include "NwGtpv2c.h"
#include "NwGtpv2cMsgParser.h"

#ifndef __NW_SAE_GW_UE__
#define __NW_SAE_GW_UE__

typedef enum
{
  NW_SAE_GW_UE_STATE_INIT = 0,
  NW_SAE_GW_UE_STATE_SAE_SESSION_CREATED,
  NW_SAE_GW_UE_STATE_SGW_SESSION_CREATED,
  NW_SAE_GW_UE_STATE_PGW_SESSION_CREATED,
  NW_SAE_GW_UE_STATE_PGW_SESSION_ESTABLISHED = NW_SAE_GW_UE_STATE_PGW_SESSION_CREATED,
  NW_SAE_GW_UE_STATE_SAE_SESSION_ESTABLISHED,
  NW_SAE_GW_UE_STATE_SGW_SESSION_ESTABLISHED,
  NW_SAE_GW_UE_STATE_WT_PGW_CREATE_SESSION_RSP,
  NW_SAE_GW_UE_STATE_WT_PGW_DELETE_SESSION_RSP,
  NW_SAE_GW_UE_STATE_WT_PGW_MODIFY_BEARER_RSP,  /* Modify Bearer Sent during X2 based HO with SGW relocation */
  NW_SAE_GW_UE_STATE_WT_PGW_MODIFY_BEARER_RSP2, /* Modify Bearer Sent during S1 based HO with SGW relocation */
  NW_SAE_GW_UE_STATE_END
} NwUeStateT;

typedef enum
{
  NW_SAE_GW_UE_EVENT_NULL               = 0,

  /* SAEGW SGW S11c Interface Events */
  NW_SAE_GW_UE_EVENT_SGW_GTPC_S11_CREATE_SESSION_REQ,
  NW_SAE_GW_UE_EVENT_SGW_GTPC_S11_MODIFY_BEARER_REQ,
  NW_SAE_GW_UE_EVENT_SGW_GTPC_S11_DELETE_SESSION_REQ,
  NW_SAE_GW_UE_EVENT_SGW_GTPC_S11_DELETE_SESSION_RSP,

  /* SAEGW SGW S5c Interface Events */
  NW_SAE_GW_UE_EVENT_SGW_GTPC_S5_CREATE_SESSION_RSP,
  NW_SAE_GW_UE_EVENT_SGW_GTPC_S5_MODIFY_BEARER_RSP,
  NW_SAE_GW_UE_EVENT_SGW_GTPC_S5_DELETE_SESSION_REQ,
  NW_SAE_GW_UE_EVENT_SGW_GTPC_S5_DELETE_SESSION_RSP,

  /* SAEGW PGW S5c Interface Events */
  NW_SAE_GW_UE_EVENT_PGW_GTPC_S5_CREATE_SESSION_REQ,
  NW_SAE_GW_UE_EVENT_PGW_GTPC_S5_MODIFY_BEARER_REQ,
  NW_SAE_GW_UE_EVENT_PGW_GTPC_S5_DELETE_SESSION_REQ,
  NW_SAE_GW_UE_EVENT_PGW_GTPC_S5_DELETE_SESSION_RSP,

  NW_SAE_GW_UE_EVENT_SESSION_TIMEOUT,
  NW_SAE_GW_UE_EVENT_NACK,
  NW_SAE_GW_UE_EVENT_END
} NwSaeGwUeEventT;

#define NW_SAE_GW_UE_SESSION_TYPE_SGW                   (0x01)
#define NW_SAE_GW_UE_SESSION_TYPE_PGW                   (0x02)
#define NW_SAE_GW_UE_SESSION_TYPE_SAE                   (NW_SAE_GW_UE_SESSION_TYPE_SGW | NW_SAE_GW_UE_SESSION_TYPE_PGW)

/**
 * Fully Qualified Tunnel Endpoint Identifier aka FTEID
 */
typedef struct 
{
  NwBoolT isValid;
  NwBoolT isIpv4;
  NwBoolT isIpv6;
  NwU8T   ifType;
  NwU32T  teidOrGreKey;
  NwU32T  ipv4Addr;
  NwU8T   ipv6Addr[16];
} NwSaeGwFteidT;

typedef struct
{
  NwU8T pdnType;
  NwU8T ipv4Addr[4];
} NwSaeGwPaaT;

/**
 * EPS Bearer  
 */
typedef  struct 
{
  NwU8T                         ebi;
  NwU8T                         cause;

  struct {
    NwSaeGwFteidT               fteidEnodeB;
    NwSaeGwFteidT               fteidSgw;
  } s1u;

  struct {
    NwSaeGwFteidT               fteidPgw;
    NwSaeGwFteidT               fteidSgw;
  } s5s8u;

} NwSaeGwEpsBearerT;

typedef NwPtrT NwDpeBearerHandleT;

typedef struct NwSaeGwUe
{
  NwU8T                         imsi[8];
  NwU8T                         msIsdn[8];
  NwU8T                         mei[8];

  NwU8T                         servingNetwork[3];
  NwU8T                         ratType;
  NwU8T                         selMode;
  NwU8T                         pdnType;

  struct {
    NwU8T       v[256];
    NwU16T      l;
  }                             apn;

  NwU8T                         apnRes;

  NwSaeGwPaaT                   paa;

  NwU32T                        sessionType;
  NwU32T                        hSgw;
  NwU32T                        hPgw;
  NwUeStateT                    state;

  NwGtpv2cStackHandleT          hGtpv2cStackSgwS11;
  NwGtpv2cStackHandleT          hGtpv2cStackSgwS5;
  NwGtpv2cStackHandleT          hGtpv2cStackPgwS5;

  struct {
    NwSaeGwFteidT               fteidMme;
    NwSaeGwFteidT               fteidSgw;
    NwGtpv2cTunnelHandleT       hSgwLocalTunnel;
  }                             s11cTunnel;

  struct {
    NwSaeGwFteidT               fteidPgw;
    NwSaeGwFteidT               fteidSgw;
    NwGtpv2cTunnelHandleT       hSgwLocalTunnel;
    NwGtpv2cTunnelHandleT       hPgwLocalTunnel;
  }                             s5s8cTunnel;

#define NW_SAE_GW_MAX_EPS_BEARERS               (16)
  struct {
    NwBoolT                     isValid;
    NwDpeBearerHandleT          hSgwUplink;
    NwDpeBearerHandleT          hSgwDownlink;
    NwDpeBearerHandleT          hPgwUplink;
    NwDpeBearerHandleT          hPgwDownlink;

    struct {
      NwSaeGwFteidT             fteidEnodeB;
      NwSaeGwFteidT             fteidSgw;
    } s1uTunnel;

    struct {
      NwSaeGwFteidT             fteidPgw;
      NwSaeGwFteidT             fteidSgw;
    } s5s8uTunnel;

  }                             epsBearer[NW_SAE_GW_MAX_EPS_BEARERS];


  RB_ENTRY (NwSaeGwUe)          ueSgwSessionRbtNode;                      /**< RB Tree Data Structure Node        */
  RB_ENTRY (NwSaeGwUe)          uePgwSessionRbtNode;                      /**< RB Tree Data Structure Node        */
} NwSaeGwUeT;

typedef struct
{
  NwSaeGwUeEventT event;
  void*           arg;
} NwSaeGwUeEventInfoT;

#ifdef __cplusplus
extern "C" {
#endif

NwSaeGwUeT*
nwSaeGwUeNew(NwGtpv2cStackHandleT hGtpv2cStackSgwS11, 
    NwGtpv2cStackHandleT hGtpv2cStackSgwS5, 
    NwGtpv2cStackHandleT hGtpv2cStackPgwS5);

NwRcT
nwSaeGwUeDelete(NwSaeGwUeT*);

NwRcT
nwSaeGwDecodePaa(NwSaeGwUeT* thiz, NwGtpv2cMsgHandleT hReqMsg, NwSaeGwPaaT *pPaa);

NwRcT
nwSaeGwUeUnexpectedEvent(NwSaeGwUeT* thiz, NwSaeGwUeEventInfoT* pEv);


#ifdef __cplusplus
}
#endif

#endif

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
 * @file NwSaeGwUe.c
*/

#include <stdio.h>
#include <assert.h>

#include "NwLog.h"
#include "NwMem.h"
#include "NwSaeGwUeLog.h"
#include "NwLogMgr.h"
#include "NwGtpv2c.h"
#include "NwGtpv2cMsg.h"
#include "NwGtpv2cMsgParser.h"
#include "NwGtpv2cIe.h"
#include "NwSaeGwUe.h"
#include "NwSaeGwUeState.h"
#include "NwSaeGwUlp.h"

#ifndef NW_ASSERT
#define NW_ASSERT assert
#endif 

#ifdef __cplusplus
extern "C" {
#endif

static char*
gpUeStateStr[]=
{
  "NW_SAE_GW_UE_STATE_INIT",
  "NW_SAE_GW_UE_STATE_SAE_SESSION_CREATED",
  "NW_SAE_GW_UE_STATE_SGW_SESSION_CREATED",
  "NW_SAE_GW_UE_STATE_PGW_SESSION_CREATED",
  "NW_SAE_GW_UE_STATE_SAE_SESSION_ESTABLISHED",
  "NW_SAE_GW_UE_STATE_SGW_SESSION_ESTABLISHED",
  "NW_SAE_GW_UE_STATE_WT_PGW_CREATE_SESSION_RSP",
  "NW_SAE_GW_UE_STATE_WT_PGW_DELETE_SESSION_RSP",
  "NW_SAE_GW_UE_STATE_WT_PGW_MODIFY_BEARER_RSP",  /* Modify Bearer Sent during X2 based HO with SGW relocation */
  "NW_SAE_GW_UE_STATE_WT_PGW_MODIFY_BEARER_RSP2", /* Modify Bearer Sent during S1 based HO with SGW relocation */
  "NW_SAE_GW_UE_STATE_END"
};

static char*
gpUeEventStr[] =
{
  "NW_SAE_GW_UE_EVENT_NULL",

  /* SAEGW SGW S11c Interface Events */
  "NW_SAE_GW_UE_EVENT_SGW_GTPC_S11_CREATE_SESSION_REQ",
  "NW_SAE_GW_UE_EVENT_SGW_GTPC_S11_MODIFY_BEARER_REQ",
  "NW_SAE_GW_UE_EVENT_SGW_GTPC_S11_DELETE_SESSION_REQ",
  "NW_SAE_GW_UE_EVENT_SGW_GTPC_S11_DELETE_SESSION_RSP",

  /* SAEGW SGW S5c Interface Events */
  "NW_SAE_GW_UE_EVENT_SGW_GTPC_S5_CREATE_SESSION_RSP",
  "NW_SAE_GW_UE_EVENT_SGW_GTPC_S5_MODIFY_BEARER_RSP",
  "NW_SAE_GW_UE_EVENT_SGW_GTPC_S5_DELETE_SESSION_REQ",
  "NW_SAE_GW_UE_EVENT_SGW_GTPC_S5_DELETE_SESSION_RSP",

  /* SAEGW SGW S5c Interface Events */
  "NW_SAE_GW_UE_EVENT_PGW_GTPC_S5_CREATE_SESSION_REQ",
  "NW_SAE_GW_UE_EVENT_PGW_GTPC_S5_MODIFY_BEARER_REQ",
  "NW_SAE_GW_UE_EVENT_PGW_GTPC_S5_DELETE_SESSION_REQ",
  "NW_SAE_GW_UE_EVENT_PGW_GTPC_S5_DELETE_SESSION_RSP",

  "NW_SAE_GW_UE_EVENT_SESSION_TIMEOUT",
  "NW_SAE_GW_UE_EVENT_NACK",
  "NW_SAE_GW_UE_EVENT_END"
};


NwSaeGwUeT*
nwSaeGwUeNew(NwGtpv2cStackHandleT hGtpv2cStackSgwS11, 
    NwGtpv2cStackHandleT hGtpv2cStackSgwS5, 
    NwGtpv2cStackHandleT hGtpv2cStackPgwS5)
{
  NwSaeGwUeT* thiz      = (NwSaeGwUeT*) nwMemNew (sizeof(NwSaeGwUeT));
  memset(thiz, 0, sizeof(NwSaeGwUeT));

  thiz->state                   = NW_SAE_GW_UE_STATE_INIT;
  thiz->hGtpv2cStackSgwS11      = hGtpv2cStackSgwS11;
  thiz->hGtpv2cStackSgwS5       = hGtpv2cStackSgwS5;
  thiz->hGtpv2cStackPgwS5       = hGtpv2cStackPgwS5;

  return thiz;
}

NwRcT
nwSaeGwUeDelete(NwSaeGwUeT* thiz)
{
  NwRcT rc;
  NwGtpv2cUlpApiT       ulpReq;

  if(thiz->s11cTunnel.hSgwLocalTunnel)
  {
    /*
     *  Send Message Request to Gtpv2c Stack Instance
     */
    ulpReq.apiType = NW_GTPV2C_ULP_DELETE_LOCAL_TUNNEL;
    ulpReq.apiInfo.deleteLocalTunnelInfo.hTunnel = thiz->s11cTunnel.hSgwLocalTunnel;

    rc = nwGtpv2cProcessUlpReq(thiz->hGtpv2cStackSgwS11, &ulpReq);
    NW_ASSERT( NW_OK == rc );
  }

  if(thiz->s5s8cTunnel.hSgwLocalTunnel)
  {
    /*
     *  Send Message Request to Gtpv2c Stack Instance
     */
    ulpReq.apiType = NW_GTPV2C_ULP_DELETE_LOCAL_TUNNEL;
    ulpReq.apiInfo.deleteLocalTunnelInfo.hTunnel = thiz->s5s8cTunnel.hSgwLocalTunnel;

    rc = nwGtpv2cProcessUlpReq(thiz->hGtpv2cStackSgwS5, &ulpReq);
    NW_ASSERT( NW_OK == rc );
  }

  if(thiz->s5s8cTunnel.hPgwLocalTunnel)
  {
    /*
     *  Send Message Request to Gtpv2c Stack Instance
     */
    ulpReq.apiType = NW_GTPV2C_ULP_DELETE_LOCAL_TUNNEL;
    ulpReq.apiInfo.deleteLocalTunnelInfo.hTunnel = thiz->s5s8cTunnel.hPgwLocalTunnel;

    rc = nwGtpv2cProcessUlpReq(thiz->hGtpv2cStackPgwS5, &ulpReq);
    NW_ASSERT( NW_OK == rc );
  }

  nwMemDelete((void*)thiz);

  return rc;
}

NwRcT
nwSaeGwDecodePaa(NwSaeGwUeT* thiz, NwGtpv2cMsgHandleT hReqMsg, NwSaeGwPaaT *pPaa)
{
  NwRcT rc;
  NwU8T *pPaaBuf;
  NwU16T paaBufLen;
  if((rc = nwGtpv2cMsgGetIeTlvP(hReqMsg, NW_GTPV2C_IE_PAA, NW_GTPV2C_IE_INSTANCE_ZERO, &pPaaBuf, &paaBufLen)) != NW_OK)
  {
    return rc;
  }

  pPaa->pdnType = *pPaaBuf;

  if(pPaa->pdnType == NW_PDN_TYPE_IPv4)
  {
    pPaaBuf++;
    memcpy(pPaa->ipv4Addr, pPaaBuf, 4);
    return NW_OK;
  }
  return NW_FAILURE;
}

NwRcT
nwSaeGwUeUnexpectedEvent(NwSaeGwUeT* thiz, NwSaeGwUeEventInfoT* pEv)
{
  NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Unexpected UE event %s in state %s!", gpUeEventStr[pEv->event], gpUeStateStr[thiz->state]);
  thiz->state = NW_SAE_GW_UE_STATE_END;
  return NW_OK;
}

#ifdef __cplusplus
}
#endif

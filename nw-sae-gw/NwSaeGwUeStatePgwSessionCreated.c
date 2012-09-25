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
 * @file NwSaeGwUeStateSaeSessionCreated.c
 */

#include <stdio.h>
#include <assert.h>

#include "NwTypes.h"
#include "NwError.h"
#include "NwMem.h"
#include "NwUtils.h"
#include "NwLog.h"
#include "NwLogMgr.h"
#include "NwSaeGwUeLog.h"
#include "NwSaeGwUeState.h"
#include "NwGtpv2cIe.h"
#include "NwGtpv2cMsg.h"
#include "NwGtpv2cMsgParser.h"
#include "NwSaeGwUlp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  NwU16T                        indication;
  NwSaeGwEpsBearerT             epsBearerTobeCreated;
  NwSaeGwEpsBearerT             epsBearerTobeRemoved;
} NwSaeGwUeSgwDeleteSessionRequestT;

static NwRcT
nwSaeGwUePgwParseDeleteSessionRequest(NwSaeGwUeT* thiz,
    NwGtpv2cMsgHandleT                  hReqMsg,
    NwGtpv2cErrorT                      *pError,
    NwSaeGwUeSgwDeleteSessionRequestT*  pDeleteSessionReq)
{
  return NW_OK;
}

static NwRcT
nwSaeGwUePgwSendDeleteSessionResponseToSgw(NwSaeGwUeT* thiz,
    NwGtpv2cTrxnHandleT hTrxn,
    NwGtpv2cErrorT      *pError,
    NwSaeGwUeSgwDeleteSessionRequestT *pDeleteSessReq)
{
  NwRcT rc;
  NwGtpv2cUlpApiT       ulpReq;

  rc = nwGtpv2cMsgNew( thiz->hGtpv2cStackPgwS5,
      NW_TRUE,                                                          /* TIED present*/
      NW_GTP_DELETE_SESSION_RSP,                                        /* Msg Type    */
      thiz->s5s8cTunnel.fteidSgw.teidOrGreKey,                          /* TEID        */
      0,                                                                /* Seq Number  */
      &(ulpReq.hMsg));
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeCause((ulpReq.hMsg), 0, pError->cause, NW_GTPV2C_CAUSE_BIT_NONE, pError->offendingIe.type, pError->offendingIe.instance);
  NW_ASSERT( NW_OK == rc );

  /*
   * Send Message Request to Gtpv2c Stack Instance
   */

  ulpReq.apiType                                = NW_GTPV2C_ULP_API_TRIGGERED_RSP;
  ulpReq.apiInfo.triggeredRspInfo.hTrxn         = hTrxn;

  rc = nwGtpv2cProcessUlpReq(thiz->hGtpv2cStackPgwS5, &ulpReq);
  NW_ASSERT( NW_OK == rc );

  NW_UE_LOG(NW_LOG_LEVEL_INFO, "Delete Session Response sent to peer!");
  return NW_OK;
}

static NwRcT
nwSaeGwUeHandlePgwS5DeleteSessionRequest(NwSaeGwUeT* thiz, NwSaeGwUeEventInfoT* pEv) 
{
  NwRcT                 rc;
  NwGtpv2cErrorT        error;
  NwGtpv2cUlpApiT       *pUlpApi = pEv->arg;
  NwSaeGwUeSgwDeleteSessionRequestT deleteSessReq;

  NW_UE_LOG(NW_LOG_LEVEL_INFO, "Delete Session Request received from peer!");

  /* Check if error has been detected already. */
  if(pUlpApi->apiInfo.initialReqIndInfo.error.cause != NW_GTPV2C_CAUSE_REQUEST_ACCEPTED)
  {
    /* Try to get the IMSI */
    rc = nwGtpv2cMsgGetIeTlv(pUlpApi->hMsg, NW_GTPV2C_IE_IMSI, NW_GTPV2C_IE_INSTANCE_ZERO, 8, thiz->imsi, NULL);

    NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Delete Session Request received with error cause %u for IE %u of instance %u!", (NwU32T)(pUlpApi->apiInfo.initialReqIndInfo.error.cause), pUlpApi->apiInfo.initialReqIndInfo.error.offendingIe.type, pUlpApi->apiInfo.initialReqIndInfo.error.offendingIe.instance);

    /* Send an error response message. */
    rc = nwSaeGwUePgwSendDeleteSessionResponseToSgw(thiz,
        pUlpApi->apiInfo.initialReqIndInfo.hTrxn,
        &(pUlpApi->apiInfo.initialReqIndInfo.error),
        &deleteSessReq);
    thiz->state = NW_SAE_GW_UE_STATE_END;
    return NW_OK;
  }

  /* Check if all conditional IEs have been received properly. */
  rc = nwSaeGwUePgwParseDeleteSessionRequest(thiz,
      pUlpApi->hMsg,
      &error,
      &deleteSessReq);
  if( rc != NW_OK )
  {
    switch(rc)
    {
      case NW_GTPV2C_IE_MISSING:
        {
          NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Conditional IE type '%u' instance '%u' missing!", error.offendingIe.type, error.offendingIe.instance);
          error.cause = NW_GTPV2C_CAUSE_CONDITIONAL_IE_MISSING;
        }
        break;
      case NW_GTPV2C_IE_INCORRECT:
        {
          NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Conditional IE type '%u' instance '%u' incorrect!", error.offendingIe.type, error.offendingIe.instance);
          error.cause = NW_GTPV2C_CAUSE_MANDATORY_IE_INCORRECT;
        }
        break;
      default:
        NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Unknown message parse error!");
        error.cause = 0;
        break;
    }

    /* Send an error response message. */
    rc = nwSaeGwUePgwSendDeleteSessionResponseToSgw(thiz,
        pUlpApi->apiInfo.initialReqIndInfo.hTrxn,
        &(error),
        &deleteSessReq);
    thiz->state = NW_SAE_GW_UE_STATE_END;
    return NW_OK;
  }

  /* Remove downlink data flows on Data Plane*/
  rc = nwSaeGwUlpRemoveDownlinkEpsBearer(thiz->hPgw, thiz, 5);
  NW_ASSERT( NW_OK == rc );

  /* Remove uplink data flows on Data Plane*/
  rc = nwSaeGwUlpRemoveUplinkEpsBearer(thiz->hPgw, thiz, 5);
  NW_ASSERT( NW_OK == rc );

  rc = nwSaeGwUlpPgwDeregisterUeSession(thiz->hPgw, thiz);
  NW_ASSERT(NW_OK == rc);

  error.cause = NW_GTPV2C_CAUSE_REQUEST_ACCEPTED;

  /* Send an delete response message. */
  rc = nwSaeGwUePgwSendDeleteSessionResponseToSgw(thiz,
      pUlpApi->apiInfo.initialReqIndInfo.hTrxn,
      &(error),
      &deleteSessReq);
  thiz->state = NW_SAE_GW_UE_STATE_END;

  return rc;
}

NwSaeUeStateT*
nwSaeGwStatePgwSessionCreatedNew()
{
  NwRcT rc;
  NwSaeUeStateT* thiz = nwSaeGwStateNew();

  rc = nwSaeGwStateSetEventHandler(thiz, 
      NW_SAE_GW_UE_EVENT_PGW_GTPC_S5_DELETE_SESSION_REQ, 
      nwSaeGwUeHandlePgwS5DeleteSessionRequest); 
  NW_ASSERT(NW_OK == rc);

  /* TODO : Add handlers for other events */
  return thiz;
}

NwRcT
nwSaeGwStatePgwSessionCreatedDelete(NwSaeUeStateT* thiz)
{
  nwMemDelete((void*)thiz);
  return NW_OK;
}

#ifdef __cplusplus
}
#endif


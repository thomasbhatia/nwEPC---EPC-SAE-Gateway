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
 * @file NwSaeGwUeStateAwaitPgwDeleteSessionRsp.c
 */

#include <stdio.h>
#include <assert.h>

#include "NwTypes.h"
#include "NwError.h"
#include "NwUtils.h"
#include "NwLogMgr.h"
#include "NwSaeGwUeLog.h"
#include "NwSaeGwUeState.h"
#include "NwGtpv2c.h"
#include "NwGtpv2cIe.h"
#include "NwGtpv2cMsg.h"
#include "NwGtpv2cMsgParser.h"
#include "NwSaeGwUlp.h"

typedef struct
{
  /* TBD */
  NwU8T pco;
} NwSaeGwUePgwDeleteSessionResponseT;


#ifdef __cplusplus
extern "C" {
#endif

static NwRcT
nwSaeGwUeSgwSendDeleteSessionResponseToMme(NwSaeGwUeT* thiz,
    NwGtpv2cTrxnHandleT                 hTrxn,
    NwGtpv2cErrorT                      *pError,
    NwSaeGwUePgwDeleteSessionResponseT  *pDeleteSessReq)
{
  NwRcT rc;
  NwGtpv2cUlpApiT       ulpReq;

  rc = nwGtpv2cMsgNew( thiz->hGtpv2cStackSgwS11,
      NW_TRUE,                                                          /* TIED present*/
      NW_GTP_DELETE_SESSION_RSP,                                        /* Msg Type    */
      thiz->s11cTunnel.fteidMme.teidOrGreKey,                           /* TEID        */
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

  rc = nwGtpv2cProcessUlpReq(thiz->hGtpv2cStackSgwS11, &ulpReq);
  NW_ASSERT( NW_OK == rc );

  NW_UE_LOG(NW_LOG_LEVEL_INFO, "Delete Session Response sent to MME!");
  return NW_OK;
}

static NwRcT
nwSaeGwUeHandleSgwS5DeleteSessionResponse(NwSaeGwUeT* thiz, NwSaeGwUeEventInfoT* pEv) 
{
  NwRcT rc;
  NwGtpv2cUlpApiT       *pUlpApi = pEv->arg;

  rc = nwSaeGwUlpSgwDeregisterUeSession(thiz->hSgw, thiz);
  NW_ASSERT(NW_OK == rc);

  rc = nwSaeGwUeSgwSendDeleteSessionResponseToMme(thiz, pUlpApi->apiInfo.triggeredRspIndInfo.hUlpTrxn, &(pUlpApi->apiInfo.initialReqIndInfo.error), NULL);
  thiz->state = NW_SAE_GW_UE_STATE_END;

  return rc;
}

static NwRcT
nwSaeGwUeHandleSgwS5DeleteSessionResponseNack(NwSaeGwUeT* thiz, NwSaeGwUeEventInfoT* pEv) 
{
  NwRcT rc;
  NwGtpv2cUlpApiT       *pUlpApi = pEv->arg;
  NwGtpv2cErrorT        error;

  rc = nwSaeGwUlpSgwDeregisterUeSession(thiz->hSgw, thiz);
  NW_ASSERT(NW_OK == rc);

  rc = nwSaeGwUeSgwSendDeleteSessionResponseToMme(thiz, pUlpApi->apiInfo.rspFailureInfo.hUlpTrxn, &error, NULL);
  thiz->state = NW_SAE_GW_UE_STATE_END;

  return rc;
}

static NwRcT
nwSaeGwStateAwaitPgwDeleteSessionEntryAction(NwSaeGwUeT* thiz, NwSaeGwUeEventInfoT* pEv) 
{
  NwRcT rc;
  NwGtpv2cUlpApiT       ulpReq;
  NwGtpv2cUlpApiT       *pUlpApi = pEv->arg;

#if 1
  /*
   * Send Message Request to Gtpv2c Stack Instance
   */

  ulpReq.apiType = NW_GTPV2C_ULP_API_INITIAL_REQ;

  ulpReq.apiInfo.initialReqInfo.hTunnel         = thiz->s5s8cTunnel.hSgwLocalTunnel;
  ulpReq.apiInfo.initialReqInfo.hUlpTrxn        = (NwGtpv2cUlpTrxnHandleT)pUlpApi->apiInfo.initialReqIndInfo.hTrxn;
  ulpReq.apiInfo.initialReqInfo.peerIp          = htonl(thiz->s5s8cTunnel.fteidPgw.ipv4Addr);

  rc = nwGtpv2cMsgNew( thiz->hGtpv2cStackSgwS5,
      NW_TRUE,
      NW_GTP_DELETE_SESSION_REQ,
      thiz->s5s8cTunnel.fteidPgw.teidOrGreKey,
      0,
      &(ulpReq.hMsg));


  rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_EBI, 0, 0);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cProcessUlpReq(thiz->hGtpv2cStackSgwS5, &ulpReq);
  NW_ASSERT( NW_OK == rc );
#endif

  return NW_OK;
}

NwSaeUeStateT*
nwSaeGwStateAwaitPgwDeleteSessionRspNew()
{
  NwRcT rc;
  NwSaeUeStateT* thiz = nwSaeGwStateNew();

  rc = nwSaeGwStateSetEntryAction(thiz, 
      nwSaeGwStateAwaitPgwDeleteSessionEntryAction); 
  NW_ASSERT(NW_OK == rc);

  rc = nwSaeGwStateSetEventHandler(thiz, 
      NW_SAE_GW_UE_EVENT_SGW_GTPC_S5_DELETE_SESSION_RSP, 
      nwSaeGwUeHandleSgwS5DeleteSessionResponse); 
  NW_ASSERT(NW_OK == rc);

  rc = nwSaeGwStateSetEventHandler(thiz, 
      NW_SAE_GW_UE_EVENT_NACK, 
      NULL /*nwSaeGwUeHandleSgwS5DeleteSessionResponseNack*/); 
  NW_ASSERT(NW_OK == rc);

  return thiz;
}

NwRcT
nwSaeGwStateAwaitPgwDeleteSessionRspDelete(NwSaeUeStateT* thiz)
{
  return nwSaeGwStateDelete(thiz);
}

#ifdef __cplusplus
}
#endif


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
 * @file NwSaeGwUeStateSaeSessionEstablished.c
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
} NwSaeGwUeSgwModifyBearerRequestT;

typedef struct
{
  NwU16T                        indication;
  NwSaeGwEpsBearerT             epsBearerTobeCreated;
  NwSaeGwEpsBearerT             epsBearerTobeRemoved;
} NwSaeGwUeSgwDeleteSessionRequestT;


static NwRcT
nwSaeGwDecodeFteid(NwU8T* ieValue, void* arg)
{
  NwSaeGwFteidT *pFteid = (NwSaeGwFteidT*) arg;
  pFteid->isIpv4 = *(ieValue) & 0x80;
  pFteid->isIpv6 = *(ieValue) & 0x40;
  pFteid->ifType = *(ieValue) & 0x1f;

  ieValue++;

  pFteid->teidOrGreKey = ntohl(*((NwU32T*)(ieValue)));
  ieValue += 4;

  if(pFteid->isIpv4)
  {
    pFteid->ipv4Addr = ntohl(*((NwU32T*)(ieValue))); 
    ieValue += 4;
  }
  if(pFteid->isIpv6)
  {
    memcpy(pFteid->ipv6Addr, ieValue, 16);
  }
  return NW_OK;
}

static NwRcT
nwSaeGwDecodeBearerContextToBeCreated(NwSaeGwUeT* thiz, NwGtpv2cMsgHandleT hReqMsg, NwSaeGwEpsBearerT *pEpsBearer)
{
  NwRcT rc;
  NwU8T t, l, i, *v;
  NwU8T *pBufEpsBearerContext;
  NwU8T* pBufEpsBearerContextEnd;
  NwU16T epsBearerContextLength;

  NwBoolT ebiFlag = NW_FALSE;
  NwBoolT bearerQosFlag = NW_FALSE;

  rc = nwGtpv2cMsgGetIeTlvP(hReqMsg,
      NW_GTPV2C_IE_BEARER_CONTEXT,
      NW_GTPV2C_IE_INSTANCE_ZERO,
      &pBufEpsBearerContext,
      &epsBearerContextLength);
  if( NW_OK != rc ) 
  { 
    return rc; 
  }

  pBufEpsBearerContextEnd = pBufEpsBearerContext + epsBearerContextLength;

  while( pBufEpsBearerContext < pBufEpsBearerContextEnd)
  {
    t = *pBufEpsBearerContext;
    l = ntohs(*(NwU16T*)(pBufEpsBearerContext + 1));

    if(l > epsBearerContextLength + 4)
      return NW_GTPV2C_IE_INCORRECT;

    i = *(NwU8T*)(pBufEpsBearerContext + 3);
    v = (NwU8T*)(pBufEpsBearerContext + 4);
    switch(t)
    {
      case NW_GTPV2C_IE_EBI:
        {
          ebiFlag = NW_TRUE;
          pEpsBearer->ebi = *((NwU8T*)(v));
        }
        break;
      case NW_GTPV2C_IE_BEARER_TFT:
        {
          /* TODO: TFT parsing */
        }
        break;
      case NW_GTPV2C_IE_FTEID:
        {
          switch(i)
          {
            case NW_GTPV2C_IE_INSTANCE_ZERO:
              {
                rc = nwSaeGwDecodeFteid(v, &(pEpsBearer->s1u.fteidEnodeB));
              }
              break;

            case NW_GTPV2C_IE_INSTANCE_ONE:
            case NW_GTPV2C_IE_INSTANCE_FOUR:
              /* Not Supporting S4 and S12 interface for now */
            default:
              break;
          }
        }
        break;
      case NW_GTPV2C_IE_BEARER_LEVEL_QOS:
        {
          bearerQosFlag = NW_TRUE;
        }
      default:
        rc = NW_OK;
    }
    pBufEpsBearerContext += (l + 4);
  }

  return rc;
}

static NwRcT
nwSaeGwDecodeBearerContextToBeRemoved(NwSaeGwUeT *thiz, NwGtpv2cMsgHandleT hReqMsg, NwSaeGwEpsBearerT *pEpsBearer)
{
  NwRcT rc;
  NwU8T t, l, i, *v;
  NwU8T *pBufEpsBearerContext;
  NwU8T* pBufEpsBearerContextEnd;
  NwU16T epsBearerContextLength;
  NwBoolT ebiFlag = NW_FALSE;

  rc = nwGtpv2cMsgGetIeTlvP(hReqMsg,
      NW_GTPV2C_IE_BEARER_CONTEXT,
      NW_GTPV2C_IE_INSTANCE_ZERO,
      &pBufEpsBearerContext,
      &epsBearerContextLength);
  if( NW_OK == rc ) 
  { 
    pBufEpsBearerContextEnd = pBufEpsBearerContext + epsBearerContextLength;

    while( pBufEpsBearerContext < pBufEpsBearerContextEnd)
    {
      t = *pBufEpsBearerContext;
      l = ntohs(*(NwU16T*)(pBufEpsBearerContext + 1));

      if(l > epsBearerContextLength + 4)
        return NW_GTPV2C_IE_INCORRECT;

      i = *(NwU8T*)(pBufEpsBearerContext + 3);
      v = (NwU8T*)(pBufEpsBearerContext + 4);
      switch(t)
      {
        case NW_GTPV2C_IE_EBI:
          {
            ebiFlag = NW_TRUE;
            pEpsBearer->ebi = *((NwU8T*)(v));
          }
          break;
        case NW_GTPV2C_IE_FTEID:
        default:
          break;
      }
      pBufEpsBearerContext += (l + 4);
    }

    if(!ebiFlag)
      return NW_GTPV2C_IE_MISSING;
  }
  return NW_OK;
}

static NwRcT
nwSaeGwUeSgwParseModifyBearerRequest(NwSaeGwUeT* thiz,
    NwGtpv2cMsgHandleT                  hReqMsg,
    NwGtpv2cErrorT                      *pError,
    NwSaeGwUeSgwModifyBearerRequestT*  pModifyBearerReq)
{
  NwU8T *pBufEpsBearerContext;
  NwU16T epsBearerContextLength;


  NwRcT rc;
  rc = nwSaeGwDecodeBearerContextToBeCreated(thiz,
      hReqMsg,
      &pModifyBearerReq->epsBearerTobeCreated);
  if( NW_OK != rc )
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_BEARER_CONTEXT;
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO;
    return rc;
  }

  rc = nwGtpv2cMsgGetIeTlvP(hReqMsg,
      NW_GTPV2C_IE_BEARER_CONTEXT,
      NW_GTPV2C_IE_INSTANCE_ONE,
      &pBufEpsBearerContext,
      &epsBearerContextLength);
  if( NW_OK == rc )
  {
    rc = nwSaeGwDecodeBearerContextToBeRemoved(thiz,
        hReqMsg,
        &pModifyBearerReq->epsBearerTobeRemoved);
    if( NW_OK != rc )
    {
      pError->offendingIe.type    = NW_GTPV2C_IE_BEARER_CONTEXT;
      pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO;
      return rc;
    }
  }

  return NW_OK;
}

static NwRcT
nwSaeGwUeSgwParseDeleteSessionRequest(NwSaeGwUeT* thiz,
    NwGtpv2cMsgHandleT                  hReqMsg,
    NwGtpv2cErrorT                      *pError,
    NwSaeGwUeSgwDeleteSessionRequestT*  pDeleteSessionReq)
{
  return NW_OK;
}
 
static NwRcT
nwSaeGwUeSgwSendModifyBearerResponseToMme(NwSaeGwUeT* thiz,
    NwGtpv2cTrxnHandleT hTrxn,
    NwGtpv2cErrorT      *pError,
    NwSaeGwUeSgwModifyBearerRequestT *pModifyBearerReq)
{
  NwRcT rc;
  NwGtpv2cUlpApiT       ulpReq;

  rc = nwGtpv2cMsgNew( thiz->hGtpv2cStackSgwS11,
      NW_TRUE,                                                          /* TIED present*/
      NW_GTP_MODIFY_BEARER_RSP,                                         /* Msg Type    */
      thiz->s11cTunnel.fteidMme.teidOrGreKey,                           /* TEID        */
      0,                                                                /* Seq Number  */
      &(ulpReq.hMsg));
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeCause((ulpReq.hMsg), 0, pError->cause, NW_GTPV2C_CAUSE_BIT_NONE, pError->offendingIe.type, pError->offendingIe.instance);
  NW_ASSERT( NW_OK == rc );

  if(pError->cause == NW_GTPV2C_CAUSE_REQUEST_ACCEPTED)
  {
    rc = nwGtpv2cMsgGroupedIeStart((ulpReq.hMsg), NW_GTPV2C_IE_BEARER_CONTEXT, 0);
    NW_ASSERT( NW_OK == rc );

    rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_EBI, 0, 0);
    NW_ASSERT( NW_OK == rc );

    rc = nwGtpv2cMsgAddIeCause((ulpReq.hMsg), 0, pError->cause, NW_GTPV2C_CAUSE_BIT_NONE, 0, 0);
    NW_ASSERT( NW_OK == rc );

    rc = nwGtpv2cMsgAddIeFteid((ulpReq.hMsg), 
        0, 
        NW_GTPV2C_IFTYPE_S1U_SGW_GTPU, 
        ((NwU32T)(thiz->epsBearer[pModifyBearerReq->epsBearerTobeCreated.ebi].s1uTunnel.fteidSgw.teidOrGreKey)),
        ((NwU32T)(thiz->epsBearer[pModifyBearerReq->epsBearerTobeCreated.ebi].s1uTunnel.fteidSgw.ipv4Addr)),
        NULL);
    NW_ASSERT( NW_OK == rc );

    rc = nwGtpv2cMsgGroupedIeEnd((ulpReq.hMsg));
    NW_ASSERT( NW_OK == rc );
  }
  /*
   * Send Message Request to Gtpv2c Stack Instance
   */

  ulpReq.apiType                                = NW_GTPV2C_ULP_API_TRIGGERED_RSP;
  ulpReq.apiInfo.triggeredRspInfo.hTrxn         = hTrxn;

  rc = nwGtpv2cProcessUlpReq(thiz->hGtpv2cStackSgwS11, &ulpReq);
  NW_ASSERT( NW_OK == rc );

  NW_UE_LOG(NW_LOG_LEVEL_INFO, "Modify Bearer Response sent to peer!");

  return rc;
}

static NwRcT
nwSaeGwUeSgwSendDeleteSessionResponseToMme(NwSaeGwUeT* thiz,
    NwGtpv2cTrxnHandleT hTrxn,
    NwGtpv2cErrorT      *pError,
    NwSaeGwUeSgwDeleteSessionRequestT *pDeleteSessReq)
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

  NW_UE_LOG(NW_LOG_LEVEL_INFO, "Delete Session Response sent to peer!");
  return NW_OK;
}

  static NwRcT
nwSaeGwUeHandleSgwS11ModifyBearerRequest(NwSaeGwUeT* thiz, NwSaeGwUeEventInfoT* pEv) 
{
  NwRcT                 rc;
  NwGtpv2cErrorT        error;
  NwSaeGwUeSgwModifyBearerRequestT modifyBearerReq;
  NwGtpv2cUlpApiT       *pUlpApi = pEv->arg;

  NW_UE_LOG(NW_LOG_LEVEL_INFO, "Modify Bearer Request received from peer!");

  /* Check if error has been detected already. */
  if(pUlpApi->apiInfo.initialReqIndInfo.error.cause != NW_GTPV2C_CAUSE_REQUEST_ACCEPTED)
  {
    /* Try to get the IMSI */
    rc = nwGtpv2cMsgGetIeTlv(pUlpApi->hMsg, NW_GTPV2C_IE_IMSI, NW_GTPV2C_IE_INSTANCE_ZERO, 8, thiz->imsi, NULL);

    NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Modify Bearer Request received with error cause %u for IE %u of instance %u!", (NwU32T)(pUlpApi->apiInfo.initialReqIndInfo.error.cause), pUlpApi->apiInfo.initialReqIndInfo.error.offendingIe.type, pUlpApi->apiInfo.initialReqIndInfo.error.offendingIe.instance);

    /* Send an error response message. */
    rc = nwSaeGwUeSgwSendModifyBearerResponseToMme(thiz,
        pUlpApi->apiInfo.initialReqIndInfo.hTrxn,
        &(pUlpApi->apiInfo.initialReqIndInfo.error),
        &modifyBearerReq);
    thiz->state = NW_SAE_GW_UE_STATE_END;
    return NW_OK;
  }

  /* Check if all conditional IEs have been received properly. */
  rc = nwSaeGwUeSgwParseModifyBearerRequest(thiz,
      pUlpApi->hMsg,
      &error,
      &modifyBearerReq);
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
    rc = nwSaeGwUeSgwSendModifyBearerResponseToMme(thiz,
        pUlpApi->apiInfo.initialReqIndInfo.hTrxn,
        &(pUlpApi->apiInfo.initialReqIndInfo.error),
        &modifyBearerReq);
    thiz->state = NW_SAE_GW_UE_STATE_END;
    return NW_OK;
  }
  else
  {
    if( modifyBearerReq.epsBearerTobeCreated.ebi < 4 ||
        thiz->epsBearer[modifyBearerReq.epsBearerTobeCreated.ebi].isValid != NW_TRUE)
    {
      error.cause = NW_GTPV2C_CAUSE_REQUEST_REJECTED;

      /* Send Create Session Response with Failure */
      rc = nwSaeGwUeSgwSendModifyBearerResponseToMme(thiz,
          pUlpApi->apiInfo.initialReqIndInfo.hTrxn,
          &(pUlpApi->apiInfo.initialReqIndInfo.error),
          &modifyBearerReq);
      NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Failed to modify UE bearer session! %u:%u", modifyBearerReq.epsBearerTobeCreated.ebi, thiz->epsBearer[modifyBearerReq.epsBearerTobeCreated.ebi].isValid);
      thiz->state = NW_SAE_GW_UE_STATE_END;
      return NW_OK;
    }

    thiz->epsBearer[modifyBearerReq.epsBearerTobeCreated.ebi].s1uTunnel.fteidEnodeB =  modifyBearerReq.epsBearerTobeCreated.s1u.fteidEnodeB;

    /* Modify downlink data flows on Data Plane*/
    rc = nwSaeGwUlpModifyDownlinkEpsBearer(thiz->hSgw, thiz, modifyBearerReq.epsBearerTobeCreated.ebi);
    NW_ASSERT( NW_OK == rc );

    thiz->epsBearer[modifyBearerReq.epsBearerTobeCreated.ebi].s1uTunnel.fteidEnodeB = 
      modifyBearerReq.epsBearerTobeCreated.s1u.fteidEnodeB; 

    error.cause = NW_GTPV2C_CAUSE_REQUEST_ACCEPTED;

    /* Send Create Session Response with success*/
    rc = nwSaeGwUeSgwSendModifyBearerResponseToMme(thiz,
        pUlpApi->apiInfo.initialReqIndInfo.hTrxn,
        &(pUlpApi->apiInfo.initialReqIndInfo.error),
        &modifyBearerReq);

    NW_UE_LOG(NW_LOG_LEVEL_NOTI, "UE Session with IP " NW_IPV4_ADDR " and IMSI %llx established successfully!", NW_IPV4_ADDR_FORMATP((thiz->paa.ipv4Addr)), NW_NTOHLL(*((NwU64T*)(thiz->imsi))));
  }

  return NW_OK;
}

  static NwRcT
nwSaeGwUeHandleSgwS11DeleteSessionRequest(NwSaeGwUeT* thiz, NwSaeGwUeEventInfoT* pEv) 
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
    rc = nwSaeGwUeSgwSendDeleteSessionResponseToMme(thiz,
        pUlpApi->apiInfo.initialReqIndInfo.hTrxn,
        &(pUlpApi->apiInfo.initialReqIndInfo.error),
        &deleteSessReq);
    thiz->state = NW_SAE_GW_UE_STATE_END;
    return NW_OK;
  }

  /* Check if all conditional IEs have been received properly. */
  rc = nwSaeGwUeSgwParseDeleteSessionRequest(thiz,
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
    rc = nwSaeGwUeSgwSendDeleteSessionResponseToMme(thiz,
        pUlpApi->apiInfo.initialReqIndInfo.hTrxn,
        &(error),
        &deleteSessReq);
    thiz->state = NW_SAE_GW_UE_STATE_END;
    return NW_OK;
  }

  /* Remove downlink data flows on Data Plane*/
  rc = nwSaeGwUlpRemoveDownlinkEpsBearer(thiz->hSgw, thiz, 5);
  NW_ASSERT( NW_OK == rc );

  /* Remove uplink data flows on Data Plane*/
  rc = nwSaeGwUlpRemoveUplinkEpsBearer(thiz->hSgw, thiz, 5);
  NW_ASSERT( NW_OK == rc );

  rc = nwSaeGwUlpSgwDeregisterUeSession(thiz->hSgw, thiz);
  NW_ASSERT(NW_OK == rc);

  if(thiz->hPgw)
  {
    rc = nwSaeGwUlpPgwDeregisterUeSession(thiz->hPgw, thiz);
    NW_ASSERT(NW_OK == rc);
  }

  error.cause = NW_GTPV2C_CAUSE_REQUEST_ACCEPTED;

  /* Send an delete response message. */
  rc = nwSaeGwUeSgwSendDeleteSessionResponseToMme(thiz,
      pUlpApi->apiInfo.initialReqIndInfo.hTrxn,
      &(error),
      &deleteSessReq);
  thiz->state = NW_SAE_GW_UE_STATE_END;

  return rc;
}

  NwSaeUeStateT*
nwSaeGwStateSaeSessionEstablishedNew()
{
  NwRcT rc;
  NwSaeUeStateT* thiz = nwSaeGwStateNew();

  rc = nwSaeGwStateSetEventHandler(thiz, 
      NW_SAE_GW_UE_EVENT_SGW_GTPC_S11_MODIFY_BEARER_REQ, 
      nwSaeGwUeHandleSgwS11ModifyBearerRequest); 
  NW_ASSERT(NW_OK == rc);

  rc = nwSaeGwStateSetEventHandler(thiz, 
      NW_SAE_GW_UE_EVENT_SGW_GTPC_S11_DELETE_SESSION_REQ, 
      nwSaeGwUeHandleSgwS11DeleteSessionRequest); 
  NW_ASSERT(NW_OK == rc);

  return thiz;
}

  NwRcT
nwSaeGwStateSaeSessionEstablishedDelete(NwSaeUeStateT* thiz)
{
  nwSaeGwStateDelete(thiz);
  return NW_OK;
}

#ifdef __cplusplus
}
#endif


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
 * @file NwSaeGwUeStateAwaitPgwModifyBearerRsp2.c
 */

#include <stdio.h>
#include <assert.h>

#include "NwTypes.h"
#include "NwError.h"
#include "NwUtils.h"
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
  NwGtpv2cErrorT                error;
  NwSaeGwEpsBearerT             epsBearerCreated;
  NwSaeGwEpsBearerT             epsBearerRemoved;
} NwSaeGwUePgwModifyBearerRsp2T;

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
            case NW_GTPV2C_IE_INSTANCE_TWO:
              {
                rc = nwSaeGwDecodeFteid(v, &(pEpsBearer->s5s8u.fteidPgw));
              }
              break;
            default:
              /* Not Supporting S4 and S12 interface for now */
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

  if(ebiFlag)
    return NW_OK;
  return NW_GTPV2C_IE_INCORRECT;
}

static NwRcT
nwSaeGwDecodeBearerContextToBeRemoved(NwU8T ieType, NwU8T ieLength, NwU8T ieInstance, NwU8T* ieValue, void* arg)
{
  NwU8T* ieEnd;
  NwU8T t, l, i, *v;
  NwSaeGwEpsBearerT *pEpsBearer = (NwSaeGwEpsBearerT*) arg;
  ieEnd = ieValue + ieLength;

  NwBoolT ebiFlag = NW_FALSE;

  while( ieValue < ieEnd )
  {
    t = *ieValue;
    l = ntohs(*(NwU16T*)(ieValue + 1));

    if(l > ieLength + 4)
      return NW_FAILURE;

    i = *(NwU8T*)(ieValue + 3);
    v = (NwU8T*)(ieValue + 4);
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
    ieValue += (l + 4);
  }

  if(ebiFlag)
    return NW_OK;
  return NW_FAILURE;

}

static NwRcT
nwSaeGwUeSgwSendModifyBearerResponseToMme(NwSaeGwUeT* thiz, 
    NwGtpv2cTrxnHandleT hTrxn, 
    NwU8T               causeValue, 
    NwU8T               ebiCreated,
    NwU8T               ebiRemoved)
{
  NwRcT rc;
  NwGtpv2cUlpApiT       ulpReq;

  /*
   * Send Message Request to Gtpv2c Stack Instance
   */

  ulpReq.apiType = NW_GTPV2C_ULP_API_TRIGGERED_RSP;

  ulpReq.apiInfo.triggeredRspInfo.hUlpTunnel    = (NwGtpv2cUlpTrxnHandleT)thiz;
  ulpReq.apiInfo.triggeredRspInfo.teidLocal     = (NwU32T) thiz;
  ulpReq.apiInfo.triggeredRspInfo.hTrxn         = hTrxn;

  rc = nwGtpv2cMsgNew( thiz->hGtpv2cStackSgwS11,
      NW_TRUE,                                                          /* TIED present*/
      NW_GTP_MODIFY_BEARER_RSP,                                         /* Msg Type    */
      0,                                                                /* TEID        */
      0,                                                                /* Seq Number  */
      &(ulpReq.hMsg));
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeCause((ulpReq.hMsg), 0, causeValue, NW_GTPV2C_CAUSE_BIT_NONE, 0, 0);
  NW_ASSERT( NW_OK == rc );

  if(causeValue == NW_GTPV2C_CAUSE_REQUEST_ACCEPTED)
  {

    ulpReq.apiType                              |= NW_GTPV2C_ULP_API_FLAG_CREATE_LOCAL_TUNNEL;

    rc = nwGtpv2cMsgAddIe((ulpReq.hMsg), NW_GTPV2C_IE_PAA, sizeof(thiz->paa), 0, (NwU8T*)&thiz->paa);
    NW_ASSERT( NW_OK == rc );

    /* TODO : Get value of control plane IPv4 address from SGW handle */
    rc = nwGtpv2cMsgAddIeFteid((ulpReq.hMsg), 
        0, 
        NW_GTPV2C_IFTYPE_S11S4_SGW_GTPC, 
        ((NwU32T)thiz), 
        thiz->s11cTunnel.fteidSgw.ipv4Addr, 
        NULL);
    NW_ASSERT( NW_OK == rc );

    /* TODO : Get value of control plane IPv4 address from PGW handle */
    rc = nwGtpv2cMsgAddIeFteid((ulpReq.hMsg), 
        1, 
        NW_GTPV2C_IFTYPE_S5S8_PGW_GTPC, 
        ((NwU32T)thiz), 
        thiz->s5s8cTunnel.fteidPgw.ipv4Addr, 
        NULL);
    NW_ASSERT( NW_OK == rc );

    rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_APN_RESTRICTION, 0, 0);
    NW_ASSERT( NW_OK == rc );


    /* Start - Encoding of grouped IE "bearer context created" */

    rc = nwGtpv2cMsgGroupedIeStart((ulpReq.hMsg), NW_GTPV2C_IE_BEARER_CONTEXT, 0);
    NW_ASSERT( NW_OK == rc );

    rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_EBI, 0, 5);
    NW_ASSERT( NW_OK == rc );

    rc = nwGtpv2cMsgAddIeCause((ulpReq.hMsg), 0, causeValue, NW_GTPV2C_CAUSE_BIT_NONE, 0, 0);
    NW_ASSERT( NW_OK == rc );

    rc = nwGtpv2cMsgAddIeFteid((ulpReq.hMsg), 
        0, 
        NW_GTPV2C_IFTYPE_S1U_SGW_GTPU, 
        ((NwU32T)(thiz->epsBearer[ebiCreated].s1uTunnel.fteidSgw.teidOrGreKey)), 
        ((NwU32T)(thiz->epsBearer[ebiCreated].s1uTunnel.fteidSgw.ipv4Addr)),
        NULL);
    NW_ASSERT( NW_OK == rc );

    rc = nwGtpv2cMsgAddIeFteid((ulpReq.hMsg), 
        2, 
        NW_GTPV2C_IFTYPE_S5S8_PGW_GTPU, 
        ((NwU32T)thiz->epsBearer[ebiCreated].s5s8uTunnel.fteidPgw.teidOrGreKey), 
        ((NwU32T)(thiz->epsBearer[ebiCreated].s5s8uTunnel.fteidPgw.ipv4Addr)), 
        NULL);
    NW_ASSERT( NW_OK == rc );

    rc = nwGtpv2cMsgGroupedIeEnd((ulpReq.hMsg));
    NW_ASSERT( NW_OK == rc );

    /* End - Encoding of grouped IE "bearer context created" */
  }
  else
  {
    NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Create Session Request rejected by PGW!");
  }

  rc = nwGtpv2cMsgSetTeid((ulpReq.hMsg), thiz->s11cTunnel.fteidMme.teidOrGreKey);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cProcessUlpReq(thiz->hGtpv2cStackSgwS11, &ulpReq);
  NW_ASSERT( NW_OK == rc );

  thiz->s11cTunnel.hSgwLocalTunnel = ulpReq.apiInfo.triggeredRspInfo.hTunnel;

  thiz->state = NW_SAE_GW_UE_STATE_SGW_SESSION_CREATED;

  NW_UE_LOG(NW_LOG_LEVEL_INFO, "Create Session Response sent to MME!");

  return rc;
}


static NwRcT
nwSaeGwUePgwCheckIeCreateSessionRequest(NwSaeGwUeT* thiz, NwU32T hReqMsg) 
{
  NwRcT rc;
  /* Check if all conditional IEs have been received properly. */

  if((rc = nwGtpv2cMsgGetIeTlv(hReqMsg, NW_GTPV2C_IE_IMSI, NW_GTPV2C_IE_INSTANCE_ZERO, 8, thiz->imsi, NULL)) != NW_OK)
  {
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTlv(hReqMsg, NW_GTPV2C_IE_MSISDN, NW_GTPV2C_IE_INSTANCE_ZERO, 8, thiz->msIsdn, NULL)) != NW_OK)
  {
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTlv(hReqMsg, NW_GTPV2C_IE_MEI, NW_GTPV2C_IE_INSTANCE_ZERO, 8, thiz->mei, NULL)) != NW_OK)
  {
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTlv(hReqMsg, NW_GTPV2C_IE_SERVING_NETWORK, NW_GTPV2C_IE_INSTANCE_ZERO, 3, (NwU8T*)&thiz->servingNetwork, NULL)) != NW_OK)
  {
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTV1(hReqMsg, NW_GTPV2C_IE_RAT_TYPE, NW_GTPV2C_IE_INSTANCE_ZERO, &thiz->ratType)) != NW_OK)
  {
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTV1(hReqMsg, NW_GTPV2C_IE_SELECTION_MODE, NW_GTPV2C_IE_INSTANCE_ZERO, &thiz->selMode)) != NW_OK)
  {
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTV1(hReqMsg, NW_GTPV2C_IE_PDN_TYPE, NW_GTPV2C_IE_INSTANCE_ZERO, &thiz->pdnType)) != NW_OK)
  {
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTV1(hReqMsg, NW_GTPV2C_IE_PAA, NW_GTPV2C_IE_INSTANCE_ZERO, (NwU8T*)&thiz->paa)) != NW_OK)
  {
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTlv(hReqMsg, NW_GTPV2C_IE_APN, NW_GTPV2C_IE_INSTANCE_ZERO, 256, thiz->apn.v, &thiz->apn.l)) != NW_OK)
  {
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTV1(hReqMsg, NW_GTPV2C_IE_APN_RESTRICTION, NW_GTPV2C_IE_INSTANCE_ZERO, &thiz->apnRes)) != NW_OK)
  {
    return rc;
  }

  return rc;
}

static NwRcT
nwSaeGwUeSgwCheckIeCreateSessionRequest(NwSaeGwUeT* thiz, NwU32T hReqMsg) 
{
  NwRcT rc;

  rc = nwGtpv2cMsgGetIeFteid(hReqMsg, 
      NW_GTPV2C_IE_INSTANCE_ZERO, 
      &thiz->s11cTunnel.fteidMme.ifType, 
      &thiz->s11cTunnel.fteidMme.teidOrGreKey, 
      &thiz->s11cTunnel.fteidMme.ipv4Addr, 
      &thiz->s11cTunnel.fteidMme.ipv6Addr[0]);
  NW_ASSERT(rc == NW_OK);

  rc = nwGtpv2cMsgGetIeFteid(hReqMsg, 
      NW_GTPV2C_IE_INSTANCE_ONE, 
      &thiz->s5s8cTunnel.fteidPgw.ifType, 
      &thiz->s5s8cTunnel.fteidPgw.teidOrGreKey, 
      &thiz->s5s8cTunnel.fteidPgw.ipv4Addr, 
      &thiz->s5s8cTunnel.fteidPgw.ipv6Addr[0]);
  NW_ASSERT(rc == NW_OK);

  if((rc = nwGtpv2cMsgGetIeTlv(hReqMsg, NW_GTPV2C_IE_IMSI, NW_GTPV2C_IE_INSTANCE_ZERO, 8, thiz->imsi, NULL)) != NW_OK)
  {
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTlv(hReqMsg, NW_GTPV2C_IE_MSISDN, NW_GTPV2C_IE_INSTANCE_ZERO, 8, thiz->msIsdn, NULL)) != NW_OK)
  {
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTlv(hReqMsg, NW_GTPV2C_IE_MEI, NW_GTPV2C_IE_INSTANCE_ZERO, 8, thiz->mei, NULL)) != NW_OK)
  {
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTlv(hReqMsg, NW_GTPV2C_IE_SERVING_NETWORK, NW_GTPV2C_IE_INSTANCE_ZERO, 3, (NwU8T*)&thiz->servingNetwork, NULL)) != NW_OK)
  {
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTV1(hReqMsg, NW_GTPV2C_IE_RAT_TYPE, NW_GTPV2C_IE_INSTANCE_ZERO, &thiz->ratType)) != NW_OK)
  {
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTV1(hReqMsg, NW_GTPV2C_IE_SELECTION_MODE, NW_GTPV2C_IE_INSTANCE_ZERO, &thiz->selMode)) != NW_OK)
  {
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTV1(hReqMsg, NW_GTPV2C_IE_PDN_TYPE, NW_GTPV2C_IE_INSTANCE_ZERO, &thiz->pdnType)) != NW_OK)
  {
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTV1(hReqMsg, NW_GTPV2C_IE_PAA, NW_GTPV2C_IE_INSTANCE_ZERO, (NwU8T*)&thiz->paa)) != NW_OK)
  {
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTlv(hReqMsg, NW_GTPV2C_IE_APN, NW_GTPV2C_IE_INSTANCE_ZERO, 256, thiz->apn.v, &thiz->apn.l)) != NW_OK)
  {
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTV1(hReqMsg, NW_GTPV2C_IE_APN_RESTRICTION, NW_GTPV2C_IE_INSTANCE_ZERO, &thiz->apnRes)) != NW_OK)
  {
    return rc;
  }

  return rc;
}

static NwRcT
nwSaeGwUeSgwParseModifyBearerResponse(NwSaeGwUeT* thiz,
    NwGtpv2cMsgHandleT                  hReqMsg,
    NwGtpv2cErrorT                      *pError,
    NwSaeGwUePgwModifyBearerRsp2T* pCreateSessReq)
{
  NwRcT rc;

  if((rc = nwGtpv2cMsgGetIeCause(hReqMsg, NW_GTPV2C_IE_INSTANCE_ZERO, &pCreateSessReq->error.cause, &pCreateSessReq->error.flags, &pCreateSessReq->error.offendingIe.type , &pCreateSessReq->error.offendingIe.instance)) != NW_OK)
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_CAUSE;
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO;
    return rc;
  }

  rc = nwSaeGwDecodeBearerContextToBeCreated(thiz,
      hReqMsg,
      &pCreateSessReq->epsBearerCreated);
  if( NW_OK != rc )
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_BEARER_CONTEXT;
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO;
    return rc;
  }

  if(pCreateSessReq->error.cause != NW_GTPV2C_CAUSE_REQUEST_ACCEPTED)
  {
    /* If cause value is not ACCEPTED, stop parsing further. */
    return NW_OK;
  }

  rc = nwGtpv2cMsgGetIeFteid(hReqMsg,
      NW_GTPV2C_IE_INSTANCE_ONE,
      &thiz->s5s8cTunnel.fteidPgw.ifType,
      &thiz->s5s8cTunnel.fteidPgw.teidOrGreKey,
      &thiz->s5s8cTunnel.fteidPgw.ipv4Addr,
      &thiz->s5s8cTunnel.fteidPgw.ipv6Addr[0]);
  if( NW_OK != rc )
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_FTEID;
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO;
    return rc;
  }

  if((rc = nwSaeGwDecodePaa(thiz, hReqMsg, &thiz->paa)) != NW_OK)
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_PAA;
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO;
    return rc;
  }

  return NW_OK;
}

static NwRcT
nwSaeGwUeHandlePgwModifyBearerRsp2(NwSaeGwUeT* thiz, NwSaeGwUeEventInfoT* pEv) 
{
  NwRcT rc;
  NwGtpv2cErrorT        error;
  NwSaeGwUePgwModifyBearerRsp2T createSessRsp;

  NwGtpv2cUlpApiT *pUlpApi = pEv->arg;

  /* Check if error has been detected already. */
  if(pUlpApi->apiInfo.triggeredRspIndInfo.error.cause != NW_GTPV2C_CAUSE_REQUEST_ACCEPTED)
  {
    NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Create Session Response Message received with error cause %u for IE %u of instance %u!", (NwU32T)(pUlpApi->apiInfo.triggeredRspIndInfo.error.cause), pUlpApi->apiInfo.triggeredRspIndInfo.error.offendingIe.type, pUlpApi->apiInfo.triggeredRspIndInfo.error.offendingIe.instance);

    return NW_OK;
  }
  /* Check if all conditional IEs have been received properly. */
  rc = nwSaeGwUeSgwParseModifyBearerResponse(thiz, pUlpApi->hMsg, &error, &createSessRsp);
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
        NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Unknown message parse error %u!", rc);
        error.cause = NW_GTPV2C_CAUSE_MANDATORY_IE_INCORRECT;
        break;
    }
    return NW_OK;
  }

  thiz->epsBearer[createSessRsp.epsBearerCreated.ebi].s5s8uTunnel.fteidPgw = createSessRsp.epsBearerCreated.s5s8u.fteidPgw;

  /* Install uplink data flows on Data Plane*/
  rc = nwSaeGwUlpInstallUplinkEpsBearer(thiz->hSgw, thiz, createSessRsp.epsBearerCreated.ebi);
  NW_ASSERT( NW_OK == rc );

  thiz->state = NW_SAE_GW_UE_STATE_SGW_SESSION_ESTABLISHED;

  return nwSaeGwUeSgwSendModifyBearerResponseToMme(thiz, pUlpApi->apiInfo.triggeredRspIndInfo.hUlpTrxn, NW_GTPV2C_CAUSE_REQUEST_ACCEPTED, 5, 0);
}

static NwRcT
nwSaeGwUeHandlePgwModifyBearerRsp2Nack(NwSaeGwUeT* thiz, NwSaeGwUeEventInfoT* pEv) 
{
  NwRcT                 rc;
  NwGtpv2cUlpApiT       *pUlpApi = pEv->arg;

  rc = nwSaeGwUeSgwSendModifyBearerResponseToMme(thiz, pUlpApi->apiInfo.rspFailureInfo.hUlpTrxn, NW_GTPV2C_CAUSE_REMOTE_PEER_NOT_RESPONDING, 5, 0);

  thiz->state = NW_SAE_GW_UE_STATE_END;
  return rc;
}

NwSaeUeStateT*
nwSaeGwStateAwaitPgwModifyBearerRsp2New()
{
  NwRcT rc;
  NwSaeUeStateT* thiz = nwSaeGwStateNew();

  rc = nwSaeGwStateSetEventHandler(thiz, 
      NW_SAE_GW_UE_EVENT_SGW_GTPC_S5_MODIFY_BEARER_RSP, 
      nwSaeGwUeHandlePgwModifyBearerRsp2); 
  NW_ASSERT(NW_OK == rc);

  rc = nwSaeGwStateSetEventHandler(thiz, 
      NW_SAE_GW_UE_EVENT_NACK, 
      nwSaeGwUeHandlePgwModifyBearerRsp2Nack); 
  NW_ASSERT(NW_OK == rc);

  return thiz;
}

NwRcT
nwSaeGwStateAwaitPgwModifyBearerRsp2Delete(NwSaeUeStateT* thiz)
{
  return nwSaeGwStateDelete(thiz);
}

#ifdef __cplusplus
}
#endif


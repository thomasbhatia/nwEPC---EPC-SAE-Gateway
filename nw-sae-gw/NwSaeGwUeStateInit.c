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
 * @file NwSaeGwUeStateInit.c
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
#include "NwSaeGwUlp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  struct {
    NwBoolT                       isPresent;
    NwU16T                        value;
  } indication;

  NwSaeGwEpsBearerT             epsBearerTobeCreated;
  NwSaeGwEpsBearerT             epsBearerTobeRemoved;
} NwSaeGwUeSgwCreateSessionRequestT;

typedef struct
{
  struct {
    NwBoolT                       isPresent;
    NwU16T                        value;
  } indication;

  NwSaeGwEpsBearerT             epsBearerTobeCreated;
  NwSaeGwEpsBearerT             epsBearerTobeRemoved;
} NwSaeGwUePgwCreateSessionRequestT;

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

  pFteid->isValid = NW_TRUE;
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

            case NW_GTPV2C_IE_INSTANCE_TWO:
              {
                rc = nwSaeGwDecodeFteid(v, &(pEpsBearer->s5s8u.fteidSgw));
              }
              break;

            case NW_GTPV2C_IE_INSTANCE_THREE:
              {
                rc = nwSaeGwDecodeFteid(v, &(pEpsBearer->s5s8u.fteidPgw));
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

  if(ebiFlag && bearerQosFlag)
    return NW_OK;
  return NW_GTPV2C_IE_INCORRECT;
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
nwSaeGwUeSgwSendCreateSessionRequestToPgw(NwSaeGwUeT* thiz, NwGtpv2cUlpTrxnHandleT hTrxn) 
{
  NwRcT rc;
  NwGtpv2cUlpApiT       ulpReq;

  rc = nwGtpv2cMsgNew( thiz->hGtpv2cStackSgwS5,
      NW_TRUE,                                          /* TIED present*/
      NW_GTP_CREATE_SESSION_REQ,                        /* Msg Type    */
      0,                                                /* TEID        */
      0,                                                /* Seq Number  */
      &(ulpReq.hMsg));

  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIe((ulpReq.hMsg), NW_GTPV2C_IE_IMSI, 8, 0, thiz->imsi);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIe((ulpReq.hMsg), NW_GTPV2C_IE_MSISDN, 8, 0, thiz->msIsdn);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIe((ulpReq.hMsg), NW_GTPV2C_IE_MEI, 8, 0, thiz->mei);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_RAT_TYPE, 0, thiz->ratType);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIe((ulpReq.hMsg), NW_GTPV2C_IE_SERVING_NETWORK, 3, 0, thiz->servingNetwork);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeFteid((ulpReq.hMsg), NW_GTPV2C_IE_INSTANCE_ZERO, NW_GTPV2C_IFTYPE_S5S8_SGW_GTPC, (NwU32T)thiz, thiz->s5s8cTunnel.fteidSgw.ipv4Addr, NULL);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_SELECTION_MODE, 0, 0x02);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_PDN_TYPE, 0, NW_PDN_TYPE_IPv4);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIe((ulpReq.hMsg), NW_GTPV2C_IE_PAA, sizeof(thiz->paa), 0, (NwU8T*)&thiz->paa);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIe((ulpReq.hMsg), NW_GTPV2C_IE_APN, thiz->apn.l, NW_GTPV2C_IE_INSTANCE_ZERO, thiz->apn.v);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_APN_RESTRICTION, 0, thiz->apnRes);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgGroupedIeStart((ulpReq.hMsg), NW_GTPV2C_IE_BEARER_CONTEXT, 0);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_EBI, NW_GTPV2C_IE_INSTANCE_ZERO, 5);
  NW_ASSERT( NW_OK == rc );

  NW_ASSERT( thiz->epsBearer[5].s5s8uTunnel.fteidSgw.ipv4Addr != 0); //AMIT
  rc = nwGtpv2cMsgAddIeFteid((ulpReq.hMsg),
      NW_GTPV2C_IE_INSTANCE_TWO,
      NW_GTPV2C_IFTYPE_S5S8_SGW_GTPU,
      ((NwU32T)(thiz->epsBearer[5].s5s8uTunnel.fteidSgw.teidOrGreKey)),
      ((NwU32T)(thiz->epsBearer[5].s5s8uTunnel.fteidSgw.ipv4Addr)),
      NULL);
  NW_ASSERT( NW_OK == rc );

#pragma pack(1)
  struct {
    NwU8T arp;
    NwU8T labelQci;
    NwU8T maximumBitRateUplink[5];
    NwU8T maximumBitRateDownlink[5];
    NwU8T  guaranteedBitRateUplink[5];
    NwU8T  guaranteedBitRateDownlink[5];
  } bearerQos;
#pragma pack()

  bearerQos.arp                         = 0x01;
  bearerQos.labelQci                    = 0x01;

  memset(bearerQos.maximumBitRateUplink, 0x00,5);
  memset(bearerQos.maximumBitRateDownlink, 0x00,5);
  memset(bearerQos.guaranteedBitRateUplink, 0x00,5);
  memset(bearerQos.guaranteedBitRateDownlink, 0x00,5);


  rc = nwGtpv2cMsgAddIe((ulpReq.hMsg), NW_GTPV2C_IE_BEARER_LEVEL_QOS, sizeof(bearerQos), 0, (NwU8T*)&bearerQos);
  NW_ASSERT( NW_OK == rc );


  rc = nwGtpv2cMsgGroupedIeEnd((ulpReq.hMsg));
  NW_ASSERT( NW_OK == rc );

  /* End - Encoding of grouped IE "bearer context created" */

  /* Send Create Session Request to PGW */

  ulpReq.apiType = (NW_GTPV2C_ULP_API_INITIAL_REQ | NW_GTPV2C_ULP_API_FLAG_CREATE_LOCAL_TUNNEL);

  ulpReq.apiInfo.initialReqInfo.hTunnel         = 0;                       
  ulpReq.apiInfo.initialReqInfo.hUlpTrxn        = hTrxn;                        /* Save the trxn for Response */
  ulpReq.apiInfo.initialReqInfo.hUlpTunnel      = (NwGtpv2cUlpTrxnHandleT)thiz;
  ulpReq.apiInfo.initialReqInfo.teidLocal       = (NwGtpv2cUlpTrxnHandleT)thiz;
  ulpReq.apiInfo.initialReqInfo.peerIp          = htonl(thiz->s5s8cTunnel.fteidPgw.ipv4Addr);

  rc = nwGtpv2cProcessUlpReq(thiz->hGtpv2cStackSgwS5, &ulpReq);
  NW_ASSERT( NW_OK == rc );

  thiz->s5s8cTunnel.hSgwLocalTunnel = ulpReq.apiInfo.initialReqInfo.hTunnel;

  NW_UE_LOG(NW_LOG_LEVEL_INFO, "Create Session Request sent to PGW "NW_IPV4_ADDR"!", NW_IPV4_ADDR_FORMAT(ulpReq.apiInfo.initialReqInfo.peerIp));

  return rc;
}

static NwRcT
nwSaeGwUeSgwSendModifyBearerRequestToPgw(NwSaeGwUeT* thiz, NwSaeGwUeSgwCreateSessionRequestT *pCreateSessReq, NwGtpv2cUlpTrxnHandleT hTrxn) 
{
  NwRcT rc;
  NwGtpv2cUlpApiT       ulpReq;

  rc = nwGtpv2cMsgNew( thiz->hGtpv2cStackSgwS5,
      NW_TRUE,                                          /* TIED present*/
      NW_GTP_MODIFY_BEARER_REQ,                         /* Msg Type    */
      thiz->s5s8cTunnel.fteidPgw.teidOrGreKey,          /* TEID        */
      0,                                                /* Seq Number  */
      &(ulpReq.hMsg));

  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIe((ulpReq.hMsg), NW_GTPV2C_IE_SERVING_NETWORK, 3, 0, thiz->servingNetwork);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_RAT_TYPE, 0, thiz->ratType);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeFteid((ulpReq.hMsg), NW_GTPV2C_IE_INSTANCE_ZERO, NW_GTPV2C_IFTYPE_S5S8_SGW_GTPC, (NwU32T)thiz, thiz->s5s8cTunnel.fteidSgw.ipv4Addr, NULL);
  NW_ASSERT( NW_OK == rc );

  /* Start - Encoding of grouped IE "bearer context to be modified" */

  rc = nwGtpv2cMsgGroupedIeStart((ulpReq.hMsg), NW_GTPV2C_IE_BEARER_CONTEXT, NW_GTPV2C_IE_INSTANCE_ZERO);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_EBI, NW_GTPV2C_IE_INSTANCE_ZERO, 5);
  NW_ASSERT( NW_OK == rc );

  NW_ASSERT( thiz->epsBearer[5].s5s8uTunnel.fteidSgw.ipv4Addr != 0); //AMIT
  rc = nwGtpv2cMsgAddIeFteid((ulpReq.hMsg),
      NW_GTPV2C_IE_INSTANCE_ONE,
      NW_GTPV2C_IFTYPE_S5S8_SGW_GTPU,
      ((NwU32T)(thiz->epsBearer[5].s5s8uTunnel.fteidSgw.teidOrGreKey)),
      ((NwU32T)(thiz->epsBearer[5].s5s8uTunnel.fteidSgw.ipv4Addr)),
      NULL);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgGroupedIeEnd((ulpReq.hMsg));
  NW_ASSERT( NW_OK == rc );

  /* End - Encoding of grouped IE "bearer context to be modified" */

  /* Send Modify Bearer Request to PGW */

  ulpReq.apiType = (NW_GTPV2C_ULP_API_INITIAL_REQ | NW_GTPV2C_ULP_API_FLAG_CREATE_LOCAL_TUNNEL);

  ulpReq.apiInfo.initialReqInfo.hTunnel         = 0;                       
  ulpReq.apiInfo.initialReqInfo.hUlpTrxn        = hTrxn;                        /* Save the trxn for Response */
  ulpReq.apiInfo.initialReqInfo.hUlpTunnel      = (NwGtpv2cUlpTrxnHandleT)thiz;
  ulpReq.apiInfo.initialReqInfo.teidLocal       = (NwGtpv2cUlpTrxnHandleT)thiz;
  ulpReq.apiInfo.initialReqInfo.peerIp          = htonl(thiz->s5s8cTunnel.fteidPgw.ipv4Addr);

  rc = nwGtpv2cProcessUlpReq(thiz->hGtpv2cStackSgwS5, &ulpReq);
  NW_ASSERT( NW_OK == rc );

  thiz->s5s8cTunnel.hSgwLocalTunnel = ulpReq.apiInfo.initialReqInfo.hTunnel;

  NW_UE_LOG(NW_LOG_LEVEL_INFO, "Modify Bearer Request sent to PGW "NW_IPV4_ADDR"!", NW_IPV4_ADDR_FORMAT(ulpReq.apiInfo.initialReqInfo.peerIp));

  return rc;
}

static NwRcT
nwSaeGwUePgwSendCreateSessionResponseToSgw(NwSaeGwUeT* thiz, 
    NwGtpv2cTrxnHandleT hTrxn, 
    NwGtpv2cErrorT      *pError,
    NwSaeGwUePgwCreateSessionRequestT *pCreateSessReq)
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

  rc = nwGtpv2cMsgNew( thiz->hGtpv2cStackPgwS5,
      NW_TRUE,                                                          /* TIED present*/
      NW_GTP_CREATE_SESSION_RSP,                                        /* Msg Type    */
      thiz->s5s8cTunnel.fteidSgw.teidOrGreKey,                          /* TEID        */
      0,                                                                /* Seq Number  */
      &(ulpReq.hMsg));
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeCause((ulpReq.hMsg), 0, pError->cause, 0, pError->offendingIe.type, pError->offendingIe.instance);
  NW_ASSERT( NW_OK == rc );

  if(pError->cause == NW_GTPV2C_CAUSE_REQUEST_ACCEPTED)
  {
    rc = nwGtpv2cMsgAddIe((ulpReq.hMsg), NW_GTPV2C_IE_PAA, sizeof(thiz->paa), 0, (NwU8T*)&thiz->paa);
    NW_ASSERT( NW_OK == rc );

    rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_APN_RESTRICTION, 0, thiz->apnRes);
    NW_ASSERT( NW_OK == rc );


    /* TODO : Get value of control plane IPv4 address from PGW handle */
    rc = nwGtpv2cMsgAddIeFteid((ulpReq.hMsg), 
        1, 
        NW_GTPV2C_IFTYPE_S5S8_PGW_GTPC, 
        ((NwU32T)thiz), 
        thiz->s5s8cTunnel.fteidPgw.ipv4Addr, 
        NULL);
    NW_ASSERT( NW_OK == rc );


    /* Start - Encoding of grouped IE "bearer context created" */

    rc = nwGtpv2cMsgGroupedIeStart((ulpReq.hMsg), NW_GTPV2C_IE_BEARER_CONTEXT, 0);
    NW_ASSERT( NW_OK == rc );

    rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_EBI, 0, pCreateSessReq->epsBearerTobeCreated.ebi);
    NW_ASSERT( NW_OK == rc );

    rc = nwGtpv2cMsgAddIeCause((ulpReq.hMsg), 0, pError->cause, NW_GTPV2C_CAUSE_BIT_NONE, 0, 0);
    NW_ASSERT( NW_OK == rc );

    rc = nwGtpv2cMsgAddIeFteid((ulpReq.hMsg), 
        NW_GTPV2C_IE_INSTANCE_TWO, 
        NW_GTPV2C_IFTYPE_S5S8_PGW_GTPU, 
        ((NwU32T)thiz->epsBearer[pCreateSessReq->epsBearerTobeCreated.ebi].s5s8uTunnel.fteidPgw.teidOrGreKey), 
        ((NwU32T)(thiz->epsBearer[pCreateSessReq->epsBearerTobeCreated.ebi].s5s8uTunnel.fteidPgw.ipv4Addr)), 
        NULL);
    NW_ASSERT( NW_OK == rc );

    rc = nwGtpv2cMsgGroupedIeEnd((ulpReq.hMsg));
    NW_ASSERT( NW_OK == rc );

    /* End - Encoding of grouped IE "bearer context created" */

    ulpReq.apiType                              |= NW_GTPV2C_ULP_API_FLAG_CREATE_LOCAL_TUNNEL;
    rc = nwGtpv2cProcessUlpReq(thiz->hGtpv2cStackPgwS5, &ulpReq);
    NW_ASSERT( NW_OK == rc );

    thiz->s5s8cTunnel.hPgwLocalTunnel = ulpReq.apiInfo.triggeredRspInfo.hTunnel;
  }
  else
  {
    NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Create Session Request rejected by PGW!");
    rc = nwGtpv2cProcessUlpReq(thiz->hGtpv2cStackPgwS5, &ulpReq);
    NW_ASSERT( NW_OK == rc );
  }
  NW_UE_LOG(NW_LOG_LEVEL_INFO, "Create Session Response sent to SGW!");

  return rc;
}

static NwRcT
nwSaeGwUeSgwSendCreateSessionResponseToMme(NwSaeGwUeT* thiz, 
    NwGtpv2cTrxnHandleT hTrxn, 
    NwGtpv2cErrorT      *pError,
    NwSaeGwUeSgwCreateSessionRequestT *pCreateSessReq)
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
      NW_TRUE,                                                          /* TIED present         */
      NW_GTP_CREATE_SESSION_RSP,                                        /* Msg Type             */
      thiz->s11cTunnel.fteidMme.teidOrGreKey,                           /* TEID                 */
      0,                                                                /* SeqNum               */
      &(ulpReq.hMsg));
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeCause((ulpReq.hMsg), 0, pError->cause, 0, pError->offendingIe.type, pError->offendingIe.instance);
  NW_ASSERT( NW_OK == rc );

  if(pError->cause == NW_GTPV2C_CAUSE_REQUEST_ACCEPTED)
  {

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


    /* Start - Encoding of grouped IE "bearer context created" */

    rc = nwGtpv2cMsgGroupedIeStart((ulpReq.hMsg), NW_GTPV2C_IE_BEARER_CONTEXT, 0);
    NW_ASSERT( NW_OK == rc );

    rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_EBI, 0, 5);
    NW_ASSERT( NW_OK == rc );

    rc = nwGtpv2cMsgAddIeCause((ulpReq.hMsg), 0, pError->cause, NW_GTPV2C_CAUSE_BIT_NONE, 0, 0);
    NW_ASSERT( NW_OK == rc );

    rc = nwGtpv2cMsgAddIeFteid((ulpReq.hMsg), 
        0, 
        NW_GTPV2C_IFTYPE_S1U_SGW_GTPU, 
        ((NwU32T)(thiz->epsBearer[pCreateSessReq->epsBearerTobeCreated.ebi].s1uTunnel.fteidSgw.teidOrGreKey)), 
        ((NwU32T)(thiz->epsBearer[pCreateSessReq->epsBearerTobeCreated.ebi].s1uTunnel.fteidSgw.ipv4Addr)),
        NULL);
    NW_ASSERT( NW_OK == rc );

    rc = nwGtpv2cMsgAddIeFteid((ulpReq.hMsg), 
        2, 
        NW_GTPV2C_IFTYPE_S5S8_PGW_GTPU, 
        ((NwU32T)thiz->epsBearer[pCreateSessReq->epsBearerTobeCreated.ebi].s5s8uTunnel.fteidPgw.teidOrGreKey), 
        ((NwU32T)(thiz->epsBearer[pCreateSessReq->epsBearerTobeCreated.ebi].s5s8uTunnel.fteidPgw.ipv4Addr)), 
        NULL);
    NW_ASSERT( NW_OK == rc );

    rc = nwGtpv2cMsgGroupedIeEnd((ulpReq.hMsg));
    NW_ASSERT( NW_OK == rc );

    ulpReq.apiType                              |= NW_GTPV2C_ULP_API_FLAG_CREATE_LOCAL_TUNNEL;

    /* End - Encoding of grouped IE "bearer context created" */
    rc = nwGtpv2cProcessUlpReq(thiz->hGtpv2cStackSgwS11, &ulpReq);
    NW_ASSERT( NW_OK == rc );

    thiz->s11cTunnel.hSgwLocalTunnel = ulpReq.apiInfo.triggeredRspInfo.hTunnel;
  }
  else
  {
    NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Create Session Request rejected by SGW!");
    rc = nwGtpv2cProcessUlpReq(thiz->hGtpv2cStackSgwS11, &ulpReq);
    NW_ASSERT( NW_OK == rc );
  }

  NW_UE_LOG(NW_LOG_LEVEL_INFO, "Create Session Response sent to MME!");
  return rc;
}

static NwRcT
nwSaeGwUePgwParseCreateSessionRequest(NwSaeGwUeT* thiz, 
    NwGtpv2cMsgHandleT                  hReqMsg,
    NwGtpv2cErrorT                      *pError,
    NwSaeGwUePgwCreateSessionRequestT*  pCreateSessReq)
{
  NwRcT rc;
  NwU8T *pBufEpsBearerContext;
  NwU16T epsBearerContextLength;

  if((rc = nwGtpv2cMsgGetIeTlv(hReqMsg, NW_GTPV2C_IE_IMSI, NW_GTPV2C_IE_INSTANCE_ZERO, 8, thiz->imsi, NULL)) != NW_OK)
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_IMSI; 
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 
    return rc;
  }

  rc = nwGtpv2cMsgGetIeFteid(hReqMsg, 
      NW_GTPV2C_IE_INSTANCE_ZERO, 
      &thiz->s5s8cTunnel.fteidSgw.ifType, 
      &thiz->s5s8cTunnel.fteidSgw.teidOrGreKey, 
      &thiz->s5s8cTunnel.fteidSgw.ipv4Addr, 
      &thiz->s5s8cTunnel.fteidSgw.ipv6Addr[0]);
  if( NW_OK != rc ) 
  { 
    pError->offendingIe.type    = NW_GTPV2C_IE_FTEID; 
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 
    return rc; 
  }

  rc = nwSaeGwDecodeBearerContextToBeCreated(thiz,
      hReqMsg, 
      &pCreateSessReq->epsBearerTobeCreated);
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
        &pCreateSessReq->epsBearerTobeRemoved);
    if( NW_OK != rc ) 
    { 
      pError->offendingIe.type    = NW_GTPV2C_IE_BEARER_CONTEXT; 
      pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ONE; 
      return rc; 
    }
  }

  /* Check if all conditional IEs have been received properly. */

  if((rc = nwGtpv2cMsgGetIeTlv(hReqMsg, NW_GTPV2C_IE_MSISDN, NW_GTPV2C_IE_INSTANCE_ZERO, 8, thiz->msIsdn, NULL)) != NW_OK)
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_MSISDN; 
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTlv(hReqMsg, NW_GTPV2C_IE_MEI, NW_GTPV2C_IE_INSTANCE_ZERO, 8, thiz->mei, NULL)) != NW_OK)
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_MEI; 
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTlv(hReqMsg, NW_GTPV2C_IE_SERVING_NETWORK, NW_GTPV2C_IE_INSTANCE_ZERO, 3, (NwU8T*)&thiz->servingNetwork, NULL)) != NW_OK)
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_SERVING_NETWORK; 
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTV1(hReqMsg, NW_GTPV2C_IE_RAT_TYPE, NW_GTPV2C_IE_INSTANCE_ZERO, &thiz->ratType)) != NW_OK)
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_RAT_TYPE; 
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTV1(hReqMsg, NW_GTPV2C_IE_SELECTION_MODE, NW_GTPV2C_IE_INSTANCE_ZERO, &thiz->selMode)) != NW_OK)
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_SELECTION_MODE; 
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTV1(hReqMsg, NW_GTPV2C_IE_PDN_TYPE, NW_GTPV2C_IE_INSTANCE_ZERO, &thiz->pdnType)) != NW_OK)
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_PDN_TYPE; 
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 
    return rc;
  }

  if((rc = nwSaeGwDecodePaa(thiz, hReqMsg, &thiz->paa)) != NW_OK)
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_PAA; 
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTlv(hReqMsg, NW_GTPV2C_IE_APN, NW_GTPV2C_IE_INSTANCE_ZERO, 256, thiz->apn.v, &thiz->apn.l)) != NW_OK)
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_APN; 
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTV1(hReqMsg, NW_GTPV2C_IE_APN_RESTRICTION, NW_GTPV2C_IE_INSTANCE_ZERO, &thiz->apnRes)) != NW_OK)
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_APN_RESTRICTION; 
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 
    return rc;
  }

  /* Optional TLVs */

  rc = nwGtpv2cMsgGetIeTV2(hReqMsg, NW_GTPV2C_IE_INDICATION, NW_GTPV2C_IE_INSTANCE_ZERO, &pCreateSessReq->indication.value);
  pCreateSessReq->indication.isPresent = (rc == NW_OK ? NW_TRUE : NW_FALSE);

  return NW_OK;
}

static NwRcT
nwSaeGwUeSgwParseCreateSessionRequest(NwSaeGwUeT* thiz, 
    NwGtpv2cMsgHandleT                  hReqMsg, 
    NwGtpv2cErrorT                      *pError,
    NwSaeGwUeSgwCreateSessionRequestT*  pCreateSessReq)
{
  NwRcT rc;
  NwU8T *pBufEpsBearerContext;
  NwU16T epsBearerContextLength;

  if((rc = nwGtpv2cMsgGetIeTlv(hReqMsg, NW_GTPV2C_IE_IMSI, NW_GTPV2C_IE_INSTANCE_ZERO, 8, thiz->imsi, NULL)) != NW_OK)
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_IMSI; 
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 

    rc = nwGtpv2cMsgGetIeFteid(hReqMsg, 
        NW_GTPV2C_IE_INSTANCE_ZERO, 
        &thiz->s11cTunnel.fteidMme.ifType, 
        &thiz->s11cTunnel.fteidMme.teidOrGreKey, 
        &thiz->s11cTunnel.fteidMme.ipv4Addr, 
        &thiz->s11cTunnel.fteidMme.ipv6Addr[0]);

    return NW_GTPV2C_IE_INCORRECT;
  }

  rc = nwGtpv2cMsgGetIeFteid(hReqMsg, 
      NW_GTPV2C_IE_INSTANCE_ZERO, 
      &thiz->s11cTunnel.fteidMme.ifType, 
      &thiz->s11cTunnel.fteidMme.teidOrGreKey, 
      &thiz->s11cTunnel.fteidMme.ipv4Addr, 
      &thiz->s11cTunnel.fteidMme.ipv6Addr[0]);
  if( NW_OK != rc ) 
  { 
    pError->offendingIe.type    = NW_GTPV2C_IE_FTEID; 
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 
    return rc; 
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
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ONE; 
    return rc; 
  }

  rc = nwSaeGwDecodeBearerContextToBeCreated(thiz,
      hReqMsg,
      &pCreateSessReq->epsBearerTobeCreated);
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
        &pCreateSessReq->epsBearerTobeRemoved);
    if( NW_OK != rc ) 
    { 
      pError->offendingIe.type    = NW_GTPV2C_IE_BEARER_CONTEXT; 
      pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 
      return rc; 
    }
  }

  if((rc = nwGtpv2cMsgGetIeTV1(hReqMsg, NW_GTPV2C_IE_RAT_TYPE, NW_GTPV2C_IE_INSTANCE_ZERO, &thiz->ratType)) != NW_OK)
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_RAT_TYPE; 
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 
    return rc;
  }

  /* Check if all conditional IEs have been received properly. */

  if((rc = nwGtpv2cMsgGetIeTlv(hReqMsg, NW_GTPV2C_IE_MSISDN, NW_GTPV2C_IE_INSTANCE_ZERO, 8, thiz->msIsdn, NULL)) != NW_OK)
  {
    if(NW_GTPV2C_IE_INCORRECT == rc)
    {
      pError->offendingIe.type    = NW_GTPV2C_IE_MSISDN; 
      pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 
      return rc;
    }
  }

  if((rc = nwGtpv2cMsgGetIeTlv(hReqMsg, NW_GTPV2C_IE_MEI, NW_GTPV2C_IE_INSTANCE_ZERO, 8, thiz->mei, NULL)) != NW_OK)
  {
    if(NW_GTPV2C_IE_INCORRECT == rc)
    {
      pError->offendingIe.type    = NW_GTPV2C_IE_MEI; 
      pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 
      return rc;
    }
  }

  if((rc = nwGtpv2cMsgGetIeTlv(hReqMsg, NW_GTPV2C_IE_SERVING_NETWORK, NW_GTPV2C_IE_INSTANCE_ZERO, 3, (NwU8T*)&thiz->servingNetwork, NULL)) != NW_OK)
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_SERVING_NETWORK; 
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTV1(hReqMsg, NW_GTPV2C_IE_SELECTION_MODE, NW_GTPV2C_IE_INSTANCE_ZERO, &thiz->selMode)) != NW_OK)
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_SELECTION_MODE; 
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTV1(hReqMsg, NW_GTPV2C_IE_PDN_TYPE, NW_GTPV2C_IE_INSTANCE_ZERO, &thiz->pdnType)) != NW_OK)
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_PDN_TYPE; 
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 
    return rc;
  }

  if((rc = nwSaeGwDecodePaa(thiz, hReqMsg, &thiz->paa)) != NW_OK)
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_PAA; 
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTlv(hReqMsg, NW_GTPV2C_IE_APN, NW_GTPV2C_IE_INSTANCE_ZERO, 256, thiz->apn.v, &thiz->apn.l)) != NW_OK)
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_APN; 
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 
    return rc;
  }

  if((rc = nwGtpv2cMsgGetIeTV1(hReqMsg, NW_GTPV2C_IE_APN_RESTRICTION, NW_GTPV2C_IE_INSTANCE_ZERO, &thiz->apnRes)) != NW_OK)
  {
    pError->offendingIe.type    = NW_GTPV2C_IE_APN_RESTRICTION; 
    pError->offendingIe.instance= NW_GTPV2C_IE_INSTANCE_ZERO; 
    return rc;
  }

  /* Optional TLVs */

  rc = nwGtpv2cMsgGetIeTV2(hReqMsg, NW_GTPV2C_IE_INDICATION, NW_GTPV2C_IE_INSTANCE_ZERO, &pCreateSessReq->indication.value);
  pCreateSessReq->indication.isPresent = (rc == NW_OK ? NW_TRUE : NW_FALSE);

  return NW_OK;
}

static NwRcT
nwSaeGwUeHandlePgwS5CreateSessionRequest(NwSaeGwUeT* thiz, NwSaeGwUeEventInfoT* pEv) 
{
  NwRcT                 rc;
  NwGtpv2cErrorT        error;
  NwGtpv2cUlpApiT       *pUlpApi = pEv->arg;
  NwSaeGwUePgwCreateSessionRequestT createSessReq;

  NW_UE_LOG(NW_LOG_LEVEL_INFO, "Create Session Request received from peer!");
  /* Check if error has been detected already. */
  if(pUlpApi->apiInfo.initialReqIndInfo.error.cause != NW_GTPV2C_CAUSE_REQUEST_ACCEPTED)
  {
    NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Message received with error cause %u for IE %u of instance %u!", (NwU32T)(pUlpApi->apiInfo.initialReqIndInfo.error.cause), pUlpApi->apiInfo.initialReqIndInfo.error.offendingIe.type, pUlpApi->apiInfo.initialReqIndInfo.error.offendingIe.instance);

    /* Send an error response message. */
    rc = nwSaeGwUePgwSendCreateSessionResponseToSgw(thiz, 
        pUlpApi->apiInfo.initialReqIndInfo.hTrxn, 
        &(pUlpApi->apiInfo.initialReqIndInfo.error),
        &createSessReq);
    thiz->state = NW_SAE_GW_UE_STATE_END;
    return NW_OK;
  }

  /* Check if all conditional IEs have been received properly. */
  rc = nwSaeGwUePgwParseCreateSessionRequest(thiz, pUlpApi->hMsg, &error, &createSessReq); 
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

    /* Send an error response message. */
    rc = nwSaeGwUePgwSendCreateSessionResponseToSgw(thiz, 
        pUlpApi->apiInfo.initialReqIndInfo.hTrxn, 
        &(pUlpApi->apiInfo.initialReqIndInfo.error),
        &createSessReq);
    thiz->state = NW_SAE_GW_UE_STATE_END;
    return NW_OK;
  }
  rc = nwSaeGwUlpRegisterPgwUeSession(thiz->hPgw, thiz);

  if(NW_OK == rc)
  {
    /* Mark the EBI as valid */
    if(createSessReq.epsBearerTobeCreated.ebi > 4 && (thiz->epsBearer[createSessReq.epsBearerTobeCreated.ebi].isValid == NW_FALSE))
    {
      thiz->epsBearer[createSessReq.epsBearerTobeCreated.ebi].isValid = NW_TRUE;

      thiz->epsBearer[createSessReq.epsBearerTobeCreated.ebi].s5s8uTunnel.fteidSgw = createSessReq.epsBearerTobeCreated.s5s8u.fteidSgw;

      /* Install uplink data flows on Data Plane*/
      rc = nwSaeGwUlpInstallUplinkEpsBearer(thiz->hPgw, thiz, createSessReq.epsBearerTobeCreated.ebi);
      NW_ASSERT( NW_OK == rc );

      /* Install downlink data flows on Data Plane*/
      rc = nwSaeGwUlpInstallDownlinkEpsBearer(thiz->hPgw, thiz, createSessReq.epsBearerTobeCreated.ebi);
      NW_ASSERT( NW_OK == rc );

      error.cause = NW_GTPV2C_CAUSE_REQUEST_ACCEPTED;
      thiz->state = NW_SAE_GW_UE_STATE_PGW_SESSION_CREATED;
    }
    else
    {
      error.cause = NW_GTPV2C_CAUSE_REQUEST_REJECTED;
      NW_ASSERT( NW_OK == rc );

      NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Failed to register PGW UE Session! Invlaid EBI.");
      thiz->state = NW_SAE_GW_UE_STATE_END;
    }
  }
  else
  {
    error.cause = NW_GTPV2C_CAUSE_REQUEST_REJECTED;
    /* TODO : Send Create Session Response with Failure */;
    NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Failed to register PGW UE Session!");
    thiz->state = NW_SAE_GW_UE_STATE_END;
  }

  rc = nwSaeGwUePgwSendCreateSessionResponseToSgw(thiz, 
      pUlpApi->apiInfo.initialReqIndInfo.hTrxn, 
      &error,
      &createSessReq);

  return rc;
}

static NwRcT
nwSaeGwUeHandleSgwS11CreateSessionRequest(NwSaeGwUeT* thiz, NwSaeGwUeEventInfoT* pEv) 
{
  NwRcT                 rc;
  NwU32T                hPgw;
  NwGtpv2cErrorT        error;
  NwGtpv2cUlpApiT       *pUlpApi = pEv->arg;
  NwSaeGwUeSgwCreateSessionRequestT createSessReq;

  NW_UE_LOG(NW_LOG_LEVEL_INFO, "Create Session Request received from peer!");

  /* Check if error has been detected already. */
  if(pUlpApi->apiInfo.initialReqIndInfo.error.cause != NW_GTPV2C_CAUSE_REQUEST_ACCEPTED)
  {
    /* Try to get the IMSI */
    rc = nwGtpv2cMsgGetIeTlv(pUlpApi->hMsg, NW_GTPV2C_IE_IMSI, NW_GTPV2C_IE_INSTANCE_ZERO, 8, thiz->imsi, NULL);

    NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Message received with error cause %u for IE %u of instance %u!", (NwU32T)(pUlpApi->apiInfo.initialReqIndInfo.error.cause), pUlpApi->apiInfo.initialReqIndInfo.error.offendingIe.type, pUlpApi->apiInfo.initialReqIndInfo.error.offendingIe.instance);

    error = pUlpApi->apiInfo.initialReqIndInfo.error;
    thiz->state = NW_SAE_GW_UE_STATE_END;

    /* Send Create Session Response to MME */;
    rc = nwSaeGwUeSgwSendCreateSessionResponseToMme(thiz, 
        pUlpApi->apiInfo.initialReqIndInfo.hTrxn, 
        &error,
        &createSessReq);
    return NW_OK;
  }

  /* Check if all conditional IEs have been received properly. */
  rc = nwSaeGwUeSgwParseCreateSessionRequest(thiz, 
      pUlpApi->hMsg, 
      &error,
      &createSessReq);

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

    thiz->state = NW_SAE_GW_UE_STATE_END;
    /* Send Create Session Response to MME */;
    rc = nwSaeGwUeSgwSendCreateSessionResponseToMme(thiz, 
        pUlpApi->apiInfo.initialReqIndInfo.hTrxn, 
        &error,
        &createSessReq);
    return NW_OK;
  }

  if(createSessReq.epsBearerTobeCreated.ebi < 4 || (thiz->epsBearer[createSessReq.epsBearerTobeCreated.ebi].isValid != NW_FALSE))
  {
    error.cause = NW_GTPV2C_CAUSE_REQUEST_REJECTED;
    NW_ASSERT( NW_OK == rc );

    NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Failed to register UE Session!");
    thiz->state = NW_SAE_GW_UE_STATE_END;
    /* Send Create Session Response to MME */;
    rc = nwSaeGwUeSgwSendCreateSessionResponseToMme(thiz, 
        pUlpApi->apiInfo.initialReqIndInfo.hTrxn, 
        &error,
        &createSessReq);
    return NW_OK;
  }

  /* Mark the EBI as valid */
  thiz->epsBearer[createSessReq.epsBearerTobeCreated.ebi].isValid = NW_TRUE;

  /* Allocate SGW uplink GRE Keys on Data Plane*/
  rc = nwSaeGwUlpAllocateTeidOrGreKeys(thiz->hSgw, thiz, createSessReq.epsBearerTobeCreated.ebi);

  rc = nwSaeGwUlpRegisterSgwUeSession(thiz->hSgw, thiz, (thiz->s5s8cTunnel.fteidPgw.ipv4Addr), &hPgw);
  if(NW_OK != rc)
  {
    error.cause = NW_GTPV2C_CAUSE_REQUEST_REJECTED;
    thiz->state = NW_SAE_GW_UE_STATE_END;
    /* Send Create Session Response to MME */;
    rc = nwSaeGwUeSgwSendCreateSessionResponseToMme(thiz, 
        pUlpApi->apiInfo.initialReqIndInfo.hTrxn, 
        &error,
        &createSessReq);
    return NW_OK;
  }

  if((createSessReq.indication.isPresent) && 
     (createSessReq.indication.value & NW_GTPV2C_INDICATION_FLAG_OI))
  {
    /* 
     * TODO: A case of TAU/RAU procedure with SGW relocation or
     * Enhanced SRNS Relocation with SGW relocation or 
     * X2-based handovers with SGW relocation. 
     */
    thiz->epsBearer[createSessReq.epsBearerTobeCreated.ebi].s5s8uTunnel.fteidPgw = createSessReq.epsBearerTobeCreated.s5s8u.fteidPgw;

    rc = nwSaeGwUeSgwSendModifyBearerRequestToPgw(thiz, &createSessReq, pUlpApi->apiInfo.initialReqIndInfo.hTrxn);
    thiz->state = NW_SAE_GW_UE_STATE_WT_PGW_MODIFY_BEARER_RSP;
  }
  else
  {
    if(hPgw)
    {
      if(NW_OK != nwSaeGwUlpRegisterPgwUeSession(hPgw, thiz))
      {
        rc = nwSaeGwUlpSgwDeregisterUeSession(thiz->hSgw, thiz);
        error.cause = NW_GTPV2C_CAUSE_REQUEST_REJECTED;
        thiz->state = NW_SAE_GW_UE_STATE_END;
        /* Send Create Session Response to MME */;
        rc = nwSaeGwUeSgwSendCreateSessionResponseToMme(thiz, 
            pUlpApi->apiInfo.initialReqIndInfo.hTrxn, 
            &error,
            &createSessReq);
        return NW_OK;
      }

      thiz->hPgw                                    = hPgw;
      thiz->s5s8cTunnel.fteidPgw.teidOrGreKey       = (NwU32T) thiz;
      thiz->state                                   = NW_SAE_GW_UE_STATE_SAE_SESSION_CREATED;

      /* Allocate GRE keys for Data Plane*/
      rc = nwSaeGwUlpAllocateTeidOrGreKeys(hPgw, thiz, createSessReq.epsBearerTobeCreated.ebi);
      NW_ASSERT( NW_OK == rc );

      /* Install uplink data flows on Data Plane*/
      rc = nwSaeGwUlpInstallUplinkEpsBearer(hPgw, thiz, createSessReq.epsBearerTobeCreated.ebi);
      NW_ASSERT( NW_OK == rc );

      error.cause                                   = NW_GTPV2C_CAUSE_REQUEST_ACCEPTED;

      /* Send Create Session Response to MME */;
      rc = nwSaeGwUeSgwSendCreateSessionResponseToMme(thiz, 
          pUlpApi->apiInfo.initialReqIndInfo.hTrxn, 
          &error,
          &createSessReq);
      return rc;
    }

    /* Send Create Session Request to PGW */;
    thiz->state = NW_SAE_GW_UE_STATE_WT_PGW_CREATE_SESSION_RSP;
    rc = nwSaeGwUeSgwSendCreateSessionRequestToPgw(thiz, pUlpApi->apiInfo.initialReqIndInfo.hTrxn);
  }
  return rc;
}

NwSaeUeStateT*
nwSaeGwStateInitNew()
{
  NwRcT rc;
  NwSaeUeStateT* thiz = nwSaeGwStateNew();

  rc = nwSaeGwStateSetEventHandler(thiz, 
      NW_SAE_GW_UE_EVENT_SGW_GTPC_S11_CREATE_SESSION_REQ, 
      nwSaeGwUeHandleSgwS11CreateSessionRequest); 
  NW_ASSERT(NW_OK == rc);

  rc = nwSaeGwStateSetEventHandler(thiz, 
      NW_SAE_GW_UE_EVENT_PGW_GTPC_S5_CREATE_SESSION_REQ, 
      nwSaeGwUeHandlePgwS5CreateSessionRequest); 
  NW_ASSERT(NW_OK == rc);

  return thiz;
}

NwRcT
nwSaeGwStateInitDelete(NwSaeUeStateT* thiz)
{
  return nwSaeGwStateDelete(thiz);
}

#ifdef __cplusplus
}
#endif


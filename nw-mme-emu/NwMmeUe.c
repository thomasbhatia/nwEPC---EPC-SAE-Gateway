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
 * @file NwMmeUe.c
*/

#include <stdio.h>
#include <assert.h>

#include "NwLog.h"
#include "NwMem.h"
#include "NwMmeUeLog.h"
#include "NwTmrMgr.h"
#include "NwLogMgr.h"
#include "NwGtpv2c.h"
#include "NwGtpv2cMsg.h"
#include "NwGtpv2cMsgParser.h"
#include "NwGtpv2cIe.h"
#include "NwMmeUe.h"
#include "NwMmeUlp.h"

#ifndef NW_ASSERT
#define NW_ASSERT assert
#endif 

typedef struct {
    NwGtpv2cMsgParserT *pCreateSessionResponseParser;
} NwMmeUeMessageParserT;

static NwMmeUeMessageParserT *gpMsgParser = NULL;

#ifdef __cplusplus
extern "C" {
#endif

static void
NW_TMR_CALLBACK(nwMmeUeHandleSessionTimeout)
{
  NwRcT                         rc;
  NwMmeUeEventInfoT             eventInfo;
  NwMmeUeT                      *thiz = (NwMmeUeT*) arg;

  eventInfo.event = NW_MME_UE_EVENT_SESSION_TIMEOUT;

  rc = nwMmeUeFsmRun(thiz, &eventInfo);
  NW_ASSERT(NW_OK == rc);

  return;
}

NwRcT
nwMmeDecodeFteid(NwU8T ieType, NwU8T ieLength, NwU8T ieInstance, NwU8T* ieValue, void* arg)
{
  NwMmeFteidT *pFteid = (NwMmeFteidT*) arg;
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

NwRcT
nwMmeDecodePaa(NwU8T ieType, NwU8T ieLength, NwU8T ieInstance, NwU8T* ieValue, void* arg)
{
  NwU32T* pIpv4Addr = (NwU32T*) arg;
  *pIpv4Addr = *((NwU32T*) (ieValue + 1));
  //printf("\nReceived UE IP Addr " NW_IPV4_ADDR "\n", NW_IPV4_ADDR_FORMAT(*pIpv4Addr));
  return NW_OK;
}

static NwRcT
nwMmeDecodeBearerContextToBeCreated(NwU8T ieType, NwU8T ieLength, NwU8T ieInstance, NwU8T* ieValue, void* arg)
{
  NwRcT rc;
  NwU8T* ieEnd;
  NwU8T t, l, i, *v;
  NwMmeEpsBearerT *pEpsBearer = (NwMmeEpsBearerT*) arg;
  ieEnd = ieValue + ieLength;

  NwBoolT ebiFlag = NW_FALSE;
  NwBoolT bearerQosFlag = NW_FALSE;

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
        {
          switch(i)
          {
            case 0:
              {
                rc = nwMmeDecodeFteid(t, l, i, v, &(pEpsBearer->s1u.fteidSgw));
              }
              break;

            case 2:
              {
                rc = nwMmeDecodeFteid(t, l, i, v, &(pEpsBearer->s5s8u.fteidPgw));
              }
              break;

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
    ieValue += (l + 4);
  }

  if(ebiFlag)
    return NW_OK;
  return NW_FAILURE;
}

static NwRcT
nwGtpv2cCreateSessionRequestIeIndication(NwU8T ieType, NwU8T ieLength, NwU8T ieInstance, NwU8T* ieValue, void* arg)
{
  NwRcT rc;
  NwMmeUeT* thiz;
  NW_UE_LOG(NW_LOG_LEVEL_DEBG, "Received IE Parse Indication for of type %u, length %u, instance %u!", ieType, ieLength, ieInstance);

  thiz = arg;
  switch(ieType)
  {
    case NW_GTPV2C_IE_CAUSE:
      {
        NwU8T* pCauseValue = (NwU8T*) arg;
        *pCauseValue = *ieValue; 
      }
      break;

    case NW_GTPV2C_IE_IMSI:
      {
        memcpy(thiz->imsi, ieValue, 8);
      }
      break;
    default:
      rc = NW_OK;
  }
  return rc;
}

NwRcT
nwMmeUeSendModifyRequestToPeer(NwMmeUeT* thiz, NwU8T ebi)
{
  NwRcT rc;
  NwGtpv2cUlpApiT       ulpReq;
  NwU32T                mmeControlPlaneIpv4Addr;

  /*
   * Send Message Request to Gtpv2c Stack Instance
   */

  ulpReq.apiType = NW_GTPV2C_ULP_API_INITIAL_REQ;

  ulpReq.apiInfo.initialReqInfo.hTunnel         = (NwGtpv2cUlpTrxnHandleT)thiz->hGtpv2cTunnel;
  ulpReq.apiInfo.initialReqInfo.hUlpTrxn        = (NwGtpv2cUlpTrxnHandleT)thiz;
  ulpReq.apiInfo.initialReqInfo.teidLocal       = (NwGtpv2cUlpTrxnHandleT)thiz;
  ulpReq.apiInfo.initialReqInfo.peerIp          = thiz->sgwIpv4Addr;

  rc = nwGtpv2cMsgNew( thiz->hGtpcStack,
      NW_TRUE,
      NW_GTP_MODIFY_BEARER_REQ,
      thiz->fteidControlPeer.teidOrGreKey,
      0,
      &(ulpReq.hMsg));

  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgGroupedIeStart((ulpReq.hMsg), NW_GTPV2C_IE_BEARER_CONTEXT, 0);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_EBI, 0, 5);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeFteid((ulpReq.hMsg), 
      NW_GTPV2C_IE_INSTANCE_ZERO, 
      NW_GTPV2C_IFTYPE_S1U_ENODEB_GTPU, 
      thiz->epsBearer[ebi].s1uTunnel.fteidEnodeB.teidOrGreKey, 
      thiz->epsBearer[ebi].s1uTunnel.fteidEnodeB.ipv4Addr, 
      NULL);

  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgGroupedIeEnd((ulpReq.hMsg));
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cProcessUlpReq(thiz->hGtpcStack, &ulpReq);
  NW_ASSERT( NW_OK == rc );

  thiz->state = NW_MME_UE_STATE_MODIFY_BEARER_REQUEST_SENT;
  NW_UE_LOG(NW_LOG_LEVEL_INFO, "Modify Bearer Request sent to peer.");

  return NW_OK;
}

NwRcT
nwMmeUeSendCreateSessionRequestToPeer(NwMmeUeT* thiz)
{
  NwRcT rc;
  NwGtpv2cUlpApiT       ulpReq;
  NwU32T                mmeControlPlaneIpv4Addr;

  /*
   * Send Message Request to Gtpv2c Stack Instance
   */

  ulpReq.apiType = NW_GTPV2C_ULP_API_INITIAL_REQ;

  ulpReq.apiInfo.initialReqInfo.hTunnel         = (NwGtpv2cUlpTrxnHandleT)0;
  ulpReq.apiInfo.initialReqInfo.hUlpTrxn        = (NwGtpv2cUlpTrxnHandleT)thiz;
  ulpReq.apiInfo.initialReqInfo.teidLocal       = (NwGtpv2cUlpTrxnHandleT)thiz;
  ulpReq.apiInfo.initialReqInfo.peerIp          = thiz->sgwIpv4Addr;

  rc = nwGtpv2cMsgNew( thiz->hGtpcStack,
      NW_TRUE,
      NW_GTP_CREATE_SESSION_REQ,
      0,
      0,
      &(ulpReq.hMsg));

  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIe((ulpReq.hMsg), NW_GTPV2C_IE_IMSI, 8, 0, thiz->imsi);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIe((ulpReq.hMsg), NW_GTPV2C_IE_MSISDN, 8, 0, thiz->msIsdn);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIe((ulpReq.hMsg), NW_GTPV2C_IE_MEI, 8, 0, thiz->msIsdn);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIe((ulpReq.hMsg), NW_GTPV2C_IE_SERVING_NETWORK, 3, 0, thiz->servingNetwork);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_RAT_TYPE, 0, NW_RAT_TYPE_EUTRAN);
  NW_ASSERT( NW_OK == rc );

  rc = nwMmeUlpGetControlPlaneIpv4Addr((NwMmeUlpT*)(thiz->hMmeUlp), &mmeControlPlaneIpv4Addr);
  if ( NW_OK == rc )
  {
    rc = nwGtpv2cMsgAddIeFteid((ulpReq.hMsg), 0, NW_GTPV2C_IFTYPE_S11_MME_GTPC, (NwU32T)thiz, htonl(mmeControlPlaneIpv4Addr), NULL);
    NW_ASSERT( NW_OK == rc );
  }

  rc = nwGtpv2cMsgAddIeFteid((ulpReq.hMsg), 1, NW_GTPV2C_IFTYPE_S5S8_PGW_GTPC, 0x00000000, htonl(thiz->pgwIpv4Addr), NULL);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_SELECTION_MODE, 0, 0x02);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_PDN_TYPE, 0, NW_PDN_TYPE_IPv4);
  NW_ASSERT( NW_OK == rc );

  struct {
    NwU8T pdnType;
    NwU8T ipv4Addr[4];
  } paa;

  paa.pdnType  = NW_PDN_TYPE_IPv4;
  paa.ipv4Addr[0] = 0x00;
  paa.ipv4Addr[1] = 0x00;
  paa.ipv4Addr[2] = 0x00;
  paa.ipv4Addr[3] = 0x00;

  rc = nwGtpv2cMsgAddIe((ulpReq.hMsg), NW_GTPV2C_IE_PAA, sizeof(paa), 0, (NwU8T*)&paa);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIe((ulpReq.hMsg), NW_GTPV2C_IE_APN, strlen("grps.nwepc.com"), 0, "grps.nwepc.com");
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_APN_RESTRICTION, 0, 0);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgGroupedIeStart((ulpReq.hMsg), NW_GTPV2C_IE_BEARER_CONTEXT, 0);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_EBI, 0, 5);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeFteid((ulpReq.hMsg), 0, NW_GTPV2C_IFTYPE_S11_MME_GTPC, (NwU32T)thiz, htonl(mmeControlPlaneIpv4Addr), NULL);
  NW_ASSERT( NW_OK == rc );

#pragma pack(1)
  struct {
    NwU8T arp;
    NwU8T labelQci;
    NwU8T maximumBitRateUplink[5];
    NwU8T maximumBitRateDownlink[5];
    NwU8T guaranteedBitRateUplink[5];
    NwU8T guaranteedBitRateDownlink[5];
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

  rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_RECOVERY, 0, 0);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cProcessUlpReq(thiz->hGtpcStack, &ulpReq);
  NW_ASSERT( NW_OK == rc );

  thiz->hGtpv2cTunnel = ulpReq.apiInfo.initialReqInfo.hTunnel; 

  NW_UE_LOG(NW_LOG_LEVEL_INFO, "Create Session Request sent to peer.");
  thiz->state = NW_MME_UE_STATE_CREATE_SESSION_REQUEST_SENT;

  return NW_OK;
}

NwU32T imsiBase = 987654321;

NwMmeUeT*
nwMmeUeNew(NwGtpv2cStackHandleT hGtpv2cStack)
{
  NwU32T imsi;
  NwMmeUeT* thiz = (NwMmeUeT*) nwMemNew(sizeof(NwMmeUeT));
  thiz->state = NW_MME_UE_STATE_INIT;

  /* TODO: Set IMSI according to TBCD */

  imsi = ((NwU32T) imsiBase++);
  thiz->imsi[0] = 0x40;
  thiz->imsi[1] = 0x49;
  thiz->imsi[2] = 0x99;

  thiz->imsi[7] = ((imsi % 10) << 4); imsi /= 10;
  thiz->imsi[7] |= 0; /* Last digit shall be 0 */

  thiz->imsi[6] = ((imsi % 10) << 4); imsi /= 10;
  thiz->imsi[6] |= (imsi % 10); imsi /= 10;

  thiz->imsi[5] = ((imsi % 10) << 4); imsi /= 10;
  thiz->imsi[5] |= (imsi % 10); imsi /= 10;

  thiz->imsi[4] = ((imsi % 10) << 4); imsi /= 10;
  thiz->imsi[4] |= (imsi % 10); imsi /= 10;

  thiz->imsi[3] = ((imsi % 10) << 4); imsi /= 10;
  thiz->imsi[3] |= (imsi % 10); imsi /= 10;

  /* Set MSISDN */
  thiz->msIsdn[0] = 0x40;
  thiz->msIsdn[1] = 0x59;
  thiz->msIsdn[2] = 0x99;
  *((NwU32T*)(thiz->msIsdn + 3)) = (NwU32T) thiz;

  /* Set Serving Network */
  thiz->servingNetwork[0] = 0x04;
  thiz->servingNetwork[1] = 0x95;
  thiz->servingNetwork[2] = 0x99;

  thiz->next            = NULL;
  thiz->registeredNext  = NULL;
  thiz->deregisteredNext= NULL;
  thiz->hGtpcStack      = hGtpv2cStack;

  if(gpMsgParser == NULL)
  {
    gpMsgParser = (NwMmeUeMessageParserT*) nwMemNew(sizeof(NwMmeUeMessageParserT));
    NW_ASSERT(nwMmeUeBuildSgwCreateSessionResponseParser(thiz, &(gpMsgParser->pCreateSessionResponseParser)) == NW_OK);
  }

  return thiz;
}

NwRcT
nwMmeUeDelete(NwMmeUeT* thiz)
{
  nwMemDelete((void*) thiz);
}

NwRcT
nwMmeUeSetImsi(NwMmeUeT* thiz, NwU8T* imsi)
{
  memcpy(thiz->imsi, imsi, 8);
  return NW_OK;
}

static NwRcT
nwMmeUeHandleCreateSessionResponse(NwMmeUeT* thiz, NwGtpv2cUlpApiT *pUlpApi) 
{
  NwRcT                 rc;
  NwGtpv2cUlpApiT       ulpReq;
  NwU8T                 causeValue;
  NwU8T                 offendingIeType;
  NwU16T                offendingIeLength;
  NwMmeEpsBearerT       epsBearerTobeCreated, epsBearerTobeRemoved;

  NW_UE_LOG(NW_LOG_LEVEL_DEBG, "Create Session Response received from peer!");

  rc = nwGtpv2cMsgParserUpdateIeReadCallbackArg(gpMsgParser->pCreateSessionResponseParser, thiz);
  NW_ASSERT(NW_OK == rc);

  rc = nwGtpv2cMsgParserUpdateIe(gpMsgParser->pCreateSessionResponseParser, NW_GTPV2C_IE_CAUSE, NW_GTPV2C_IE_INSTANCE_ZERO, NW_GTPV2C_IE_PRESENCE_MANDATORY, nwGtpv2cCreateSessionRequestIeIndication, &causeValue);
  NW_ASSERT(NW_OK == rc);

  rc = nwGtpv2cMsgParserUpdateIe(gpMsgParser->pCreateSessionResponseParser, NW_GTPV2C_IE_FTEID, NW_GTPV2C_IE_INSTANCE_ZERO, NW_GTPV2C_IE_PRESENCE_CONDITIONAL, nwMmeDecodeFteid, &thiz->fteidControlPeer);
  NW_ASSERT(NW_OK == rc);

  rc = nwGtpv2cMsgParserUpdateIe(gpMsgParser->pCreateSessionResponseParser, NW_GTPV2C_IE_PAA, NW_GTPV2C_IE_INSTANCE_ZERO, NW_GTPV2C_IE_PRESENCE_CONDITIONAL, nwMmeDecodePaa, &thiz->pdnIpv4Addr);
  NW_ASSERT(NW_OK == rc);

  rc = nwGtpv2cMsgParserUpdateIe(gpMsgParser->pCreateSessionResponseParser, NW_GTPV2C_IE_BEARER_CONTEXT, NW_GTPV2C_IE_INSTANCE_ZERO, NW_GTPV2C_IE_PRESENCE_CONDITIONAL, nwMmeDecodeBearerContextToBeCreated, &epsBearerTobeCreated);
  NW_ASSERT(NW_OK == rc);
  /*
   * TODO: Parse the Create Session Response message
   */
  rc = nwGtpv2cMsgParserRun(gpMsgParser->pCreateSessionResponseParser, (pUlpApi->hMsg), &offendingIeType, (NwU8T*)&offendingIeLength);
  if( rc != NW_OK )
  {
    switch(rc)
    {
      case NW_GTPV2C_MANDATORY_IE_MISSING:
        NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Mandatory IE type '%u' instance '%u' missing!", offendingIeType, offendingIeLength);
        causeValue = NW_GTPV2C_CAUSE_MANDATORY_IE_MISSING;
        break;
      default:
        NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Unknown message parse error!");
        causeValue = 0;
        break;
    }
  }
  else
  {
    if(causeValue == NW_GTPV2C_CAUSE_REQUEST_ACCEPTED)
    {
      NW_UE_LOG(NW_LOG_LEVEL_INFO, "UE Session created successfuly!");
      thiz->state = NW_MME_UE_STATE_SESSION_CREATED;
      thiz->epsBearer[epsBearerTobeCreated.ebi].isValid                 = NW_TRUE;
      thiz->epsBearer[epsBearerTobeCreated.ebi].s1uTunnel.fteidSgw      = epsBearerTobeCreated.s1u.fteidSgw;
      thiz->epsBearer[epsBearerTobeCreated.ebi].s5s8Tunnel.fteidPgw     = epsBearerTobeCreated.s5s8u.fteidPgw;

      rc = nwMmeDpeCreateGtpuIpv4Flow(thiz->pDpe, 
          (NwU32T)thiz, 
          (NwU32T)thiz, 
          &(thiz->epsBearer[epsBearerTobeCreated.ebi].s1uTunnel.fteidEnodeB.teidOrGreKey), 
          &(thiz->epsBearer[epsBearerTobeCreated.ebi].s1uTunnel.fteidEnodeB.ipv4Addr),
          &(thiz->epsBearer[epsBearerTobeCreated.ebi].s1uTunnel.hDownlink));


      rc = nwMmeDpeCreateIpv4GtpuFlow(thiz->pDpe,
          (NwU32T)thiz,
          (thiz->epsBearer[epsBearerTobeCreated.ebi].s1uTunnel.fteidSgw.teidOrGreKey), 
          (thiz->epsBearer[epsBearerTobeCreated.ebi].s1uTunnel.fteidSgw.ipv4Addr),
          thiz->pdnIpv4Addr,
          &(thiz->epsBearer[epsBearerTobeCreated.ebi].s1uTunnel.hUplink));

      rc = nwMmeUeSendModifyRequestToPeer(thiz, epsBearerTobeCreated.ebi);
      NW_ASSERT(NW_OK == rc);
    }
    else
    {
      NW_UE_LOG(NW_LOG_LEVEL_ERRO, "UE Session created failed with error cause '%u' !", causeValue);
      thiz->state = NW_MME_UE_STATE_END;
    }
  }

  return NW_OK;
}

NwRcT
nwMmeUeBuildSgwCreateSessionResponseParser(NwMmeUeT* thiz, NwGtpv2cMsgParserT** ppMsgParser) 
{
  NwRcT                 rc;
  NwGtpv2cMsgParserT    *pMsgParser;

  /*
   * TODO: Parse the Create Session Request message
   */
  rc = nwGtpv2cMsgParserNew(thiz->hGtpcStack, NW_GTP_CREATE_SESSION_RSP, nwGtpv2cCreateSessionRequestIeIndication, NULL, &pMsgParser);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgParserAddIe(pMsgParser, NW_GTPV2C_IE_PAA, NW_GTPV2C_IE_INSTANCE_ZERO, NW_GTPV2C_IE_PRESENCE_CONDITIONAL, nwGtpv2cCreateSessionRequestIeIndication, NULL);
  NW_ASSERT(NW_OK == rc);

  rc = nwGtpv2cMsgParserAddIe(pMsgParser, NW_GTPV2C_IE_CAUSE, NW_GTPV2C_IE_INSTANCE_ZERO, NW_GTPV2C_IE_PRESENCE_MANDATORY, nwGtpv2cCreateSessionRequestIeIndication, NULL);
  NW_ASSERT(NW_OK == rc);

  rc = nwGtpv2cMsgParserAddIe(pMsgParser, NW_GTPV2C_IE_FTEID, NW_GTPV2C_IE_INSTANCE_ZERO, NW_GTPV2C_IE_PRESENCE_CONDITIONAL, nwGtpv2cCreateSessionRequestIeIndication, NULL);
  NW_ASSERT(NW_OK == rc);

  rc = nwGtpv2cMsgParserAddIe(pMsgParser, NW_GTPV2C_IE_FTEID, NW_GTPV2C_IE_INSTANCE_ONE, NW_GTPV2C_IE_PRESENCE_CONDITIONAL, nwGtpv2cCreateSessionRequestIeIndication, NULL);
  NW_ASSERT(NW_OK == rc);

  rc = nwGtpv2cMsgParserAddIe(pMsgParser, NW_GTPV2C_IE_APN_RESTRICTION, NW_GTPV2C_IE_INSTANCE_ZERO, NW_GTPV2C_IE_PRESENCE_CONDITIONAL, nwGtpv2cCreateSessionRequestIeIndication, NULL);
  NW_ASSERT(NW_OK == rc);

  rc = nwGtpv2cMsgParserAddIe(pMsgParser, NW_GTPV2C_IE_PCO, NW_GTPV2C_IE_INSTANCE_ZERO, NW_GTPV2C_IE_PRESENCE_CONDITIONAL, nwGtpv2cCreateSessionRequestIeIndication, NULL);
  NW_ASSERT(NW_OK == rc);

  rc = nwGtpv2cMsgParserAddIe(pMsgParser, NW_GTPV2C_IE_BEARER_CONTEXT, NW_GTPV2C_IE_INSTANCE_ZERO, NW_GTPV2C_IE_PRESENCE_CONDITIONAL, nwGtpv2cCreateSessionRequestIeIndication, NULL);
  NW_ASSERT(NW_OK == rc);

  rc = nwGtpv2cMsgParserAddIe(pMsgParser, NW_GTPV2C_IE_RECOVERY, NW_GTPV2C_IE_INSTANCE_ZERO, NW_GTPV2C_IE_PRESENCE_CONDITIONAL, nwGtpv2cCreateSessionRequestIeIndication, NULL);
  NW_ASSERT(NW_OK == rc);

  *ppMsgParser = pMsgParser;
  return rc;
}

NwRcT
nwMmeUeStDeleteSessionReqSentHandleGtpcMsgIndication(NwMmeUeT* thiz, NwMmeUeEventInfoT* pEv)
{
  NwRcT rc;
  NwGtpv2cUlpApiT *pUlpApi = pEv->arg;
  switch(pUlpApi->apiInfo.triggeredRspIndInfo.msgType)
  {
    case NW_GTP_DELETE_SESSION_RSP:
      {
        NwGtpv2cUlpApiT       ulpReq;
        NW_UE_LOG(NW_LOG_LEVEL_INFO, "Delete Session Response received in DeleteSessionReqSent state!");
        thiz->state = NW_MME_UE_STATE_INIT;

        if(((NwMmeUlpT*)(thiz->hMmeUlp))->pMsListDeregisteredHead == NULL)
        {
          ((NwMmeUlpT*)(thiz->hMmeUlp))->pMsListDeregisteredHead = thiz;
          ((NwMmeUlpT*)(thiz->hMmeUlp))->pMsListDeregisteredTail = thiz;
          thiz->deregisteredNext = NULL;
        }
        else
        {
          ((NwMmeUlpT*)(thiz->hMmeUlp))->pMsListDeregisteredTail->deregisteredNext = thiz;
          ((NwMmeUlpT*)(thiz->hMmeUlp))->pMsListDeregisteredTail = thiz;
          thiz->deregisteredNext = NULL;
        }

        /*
         * Send Message Request to Gtpv2c Stack Instance
         */

        ulpReq.apiType = NW_GTPV2C_ULP_DELETE_LOCAL_TUNNEL;
        ulpReq.apiInfo.deleteLocalTunnelInfo.hTunnel = thiz->hGtpv2cTunnel;

        rc = nwGtpv2cProcessUlpReq(thiz->hGtpcStack, &ulpReq);
        NW_ASSERT( NW_OK == rc );

      }
      break;
    default:
      {
        NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Unhandled message type '%u' received in DeleteSessionReqSent state!", pUlpApi->apiInfo.triggeredRspIndInfo.msgType);
        thiz->state = NW_MME_UE_STATE_END;
      }
  }
  return rc;
}

NwRcT
nwMmeUeStModifyBearerReqSentHandleGtpcMsgIndication(NwMmeUeT* thiz, NwMmeUeEventInfoT* pEv)
{
  NwRcT rc;
  NwGtpv2cUlpApiT *pUlpApi = pEv->arg;
  switch(pUlpApi->apiInfo.triggeredRspIndInfo.msgType)
  {
    case NW_GTP_MODIFY_BEARER_RSP:
      {
        NW_UE_LOG(NW_LOG_LEVEL_DEBG, "Modify Bearer Rsp received in ModifyBearerReqSent state!");
        NW_UE_LOG(NW_LOG_LEVEL_NOTI, "Session established successfully with IPv4 address "NW_IPV4_ADDR, NW_IPV4_ADDR_FORMAT(thiz->pdnIpv4Addr));
        thiz->state = NW_MME_UE_STATE_SESSION_CREATED;

        ((NwMmeUlpT*) (thiz->hMmeUlp))->sessionCount++;

        if(thiz->sessionTimeout)
        {
          /* Start Session Timeout Timer */
          rc = nwTmrMgrStartTimer(0, thiz->sessionTimeout, 0, NW_TIMER_TYPE_ONE_SHOT, nwMmeUeHandleSessionTimeout, thiz, &thiz->hSessionTimer);
          NW_ASSERT(NW_OK == rc);
        }
      }
      break;

    default:
      {
        NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Unhandled message type '%u' received in ModifyBearerReqSent state!", pUlpApi->apiInfo.triggeredRspIndInfo.msgType);
      }
  }
  return rc;
}

NwRcT
nwMmeUeStCreateSessionReqSentHandleGtpcMsgIndication(NwMmeUeT* thiz, NwMmeUeEventInfoT* pEv)
{
  NwRcT rc;
  NwGtpv2cUlpApiT *pUlpApi = pEv->arg;
  switch(pUlpApi->apiInfo.triggeredRspIndInfo.msgType)
  {
    case NW_GTP_CREATE_SESSION_RSP:
      {
        NW_UE_LOG(NW_LOG_LEVEL_DEBG, "Create Session Request received in init state!");
        rc = nwMmeUeHandleCreateSessionResponse(thiz, pUlpApi);
      }
      break;
    default:
      {
        NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Unhandled message type '%u' received in init state!", pUlpApi->apiInfo.triggeredRspIndInfo.msgType);
        thiz->state = NW_MME_UE_STATE_END;
      }
  }
  return rc;

}

NwRcT
nwMmeUeStSessionCreatedHandleSessionTimeout(NwMmeUeT* thiz, NwMmeUeEventInfoT* pEv)
{
  NwRcT rc;
  NwGtpv2cUlpApiT *pUlpApi = pEv->arg;
  NwGtpv2cUlpApiT       ulpReq;
  NwU32T                mmeControlPlaneIpv4Addr;

  /*
   * Send Message Request to Gtpv2c Stack Instance
   */

  ulpReq.apiType = NW_GTPV2C_ULP_API_INITIAL_REQ;

  ulpReq.apiInfo.initialReqInfo.hTunnel         = (NwGtpv2cUlpTrxnHandleT)thiz->hGtpv2cTunnel;
  ulpReq.apiInfo.initialReqInfo.hUlpTrxn        = (NwGtpv2cUlpTrxnHandleT)thiz;
  ulpReq.apiInfo.initialReqInfo.teidLocal       = (NwGtpv2cUlpTrxnHandleT)thiz;
  ulpReq.apiInfo.initialReqInfo.peerIp          = thiz->sgwIpv4Addr;

  rc = nwGtpv2cMsgNew( thiz->hGtpcStack,
      NW_TRUE,
      NW_GTP_DELETE_SESSION_REQ,
      thiz->fteidControlPeer.teidOrGreKey,
      0,
      &(ulpReq.hMsg));

  rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_EBI, 0, 5);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeTV2((ulpReq.hMsg), NW_GTPV2C_IE_INDICATION, 0, (0x0000 | NW_GTPV2C_INDICATION_FLAG_OI));
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cMsgAddIeTV1((ulpReq.hMsg), NW_GTPV2C_IE_RECOVERY, 0, 0);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cProcessUlpReq(thiz->hGtpcStack, &ulpReq);
  NW_ASSERT( NW_OK == rc );

  thiz->state = NW_MME_UE_STATE_DELETE_SESSION_REQUEST_SENT;
  rc = nwMmeDpeDestroyFlow(thiz->pDpe, thiz->epsBearer[5].s1uTunnel.hDownlink);
  NW_ASSERT( NW_OK == rc );
  rc = nwMmeDpeDestroyFlow(thiz->pDpe, thiz->epsBearer[5].s1uTunnel.hUplink);
  NW_ASSERT( NW_OK == rc );

  NW_UE_LOG(NW_LOG_LEVEL_INFO, "Delete Session Request sent to peer.");
  return rc;
}

NwRcT
nwMmeUeFsmRun(NwMmeUeT* thiz, NwMmeUeEventInfoT* pEv)
{
  NwRcT rc;

  NW_UE_ENTER();

  NW_ASSERT(thiz);

  switch(thiz->state)
  {
    case NW_MME_UE_STATE_INIT:
      {
        switch(pEv->event)
        {
          case NW_MME_UE_EVENT_NETWORK_ENTRY_START: 
            {
              /*
               * Send Create Session Request to SGW
               */
              rc = nwMmeUeSendCreateSessionRequestToPeer(thiz);
            }
            break;
          default:
            NW_ASSERT(0);
        }
      }
      break;

    case NW_MME_UE_STATE_CREATE_SESSION_REQUEST_SENT:
      {
        switch(pEv->event)
        {
          case NW_MME_UE_EVENT_GTPC_TRIGGERED_RSP_MSG_INDICATION:
            {
              NW_UE_LOG(NW_LOG_LEVEL_DEBG, "GTPC mesasge indication in state NW_MME_UE_STATE_CREATE_SESSION_REQUEST_SENT.");
              rc = nwMmeUeStCreateSessionReqSentHandleGtpcMsgIndication(thiz, pEv);
            }
            break;

          case NW_MME_UE_EVENT_NACK:
            {
              NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Create Session Response not received from peer!");
              rc = NW_OK;
            }
            break;
          default:
            NW_ASSERT(0);
        }
      }
      break;

    case NW_MME_UE_STATE_SESSION_CREATED:
      {
        switch(pEv->event)
        {
          case NW_MME_UE_EVENT_GTPC_MSG_INDICATION:
            {
              NW_UE_LOG(NW_LOG_LEVEL_DEBG, "GTPC mesasge indication in state NW_MME_UE_STATE_SESSION_CREATED.");
              rc = nwMmeUeStCreateSessionReqSentHandleGtpcMsgIndication(thiz, pEv);
            }
            break;

          case NW_MME_UE_EVENT_SESSION_TIMEOUT:
            {
              NW_UE_LOG(NW_LOG_LEVEL_NOTI, "UE Session session timed-out!");
              rc = nwMmeUeStSessionCreatedHandleSessionTimeout(thiz, pEv);
            }
            break;
          default:
            NW_UE_LOG(NW_LOG_LEVEL_CRIT, "Received unhandled event %u!", pEv->event);
            NW_ASSERT(0);
        }
      }
      break;

    case NW_MME_UE_STATE_MODIFY_BEARER_REQUEST_SENT:
      {
        switch(pEv->event)
        {
          case NW_MME_UE_EVENT_GTPC_TRIGGERED_RSP_MSG_INDICATION:
            {
              NW_UE_LOG(NW_LOG_LEVEL_DEBG, "GTPC mesasge indication in state NW_MME_UE_STATE_MODIFY_BEARER_REQUEST_SENT.");
              rc = nwMmeUeStModifyBearerReqSentHandleGtpcMsgIndication(thiz, pEv);
            }
            break;

          case NW_MME_UE_EVENT_NACK:
            {
              NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Modify Bearer Response not received from peer!");
              rc = NW_OK;
            }
            break;

          default:
            NW_ASSERT(0);
        }
      }
      break;


    case NW_MME_UE_STATE_DELETE_SESSION_REQUEST_SENT:
      {
        switch(pEv->event)
        {
          case NW_MME_UE_EVENT_GTPC_TRIGGERED_RSP_MSG_INDICATION:
            {
              NW_UE_LOG(NW_LOG_LEVEL_DEBG, "GTPC mesasge indication in state NW_MME_UE_STATE_SESSION_CREATED.");
              rc = nwMmeUeStDeleteSessionReqSentHandleGtpcMsgIndication(thiz, pEv);
            }
            break;

          case NW_MME_UE_EVENT_NACK:
            {
              NW_UE_LOG(NW_LOG_LEVEL_ERRO, "Delete Session Response not received from peer!");
              rc = NW_OK;
            }
            break;

          default:
            NW_ASSERT(0);
        }
      }
      break;


    default:
      {
        NW_UE_LOG(NW_LOG_LEVEL_ERRO, "UE Session in unhandled state %u!", thiz->state);
        rc = NW_OK;
      }
  }

  NW_UE_LEAVE();
  return rc;
}

#ifdef __cplusplus
}
#endif


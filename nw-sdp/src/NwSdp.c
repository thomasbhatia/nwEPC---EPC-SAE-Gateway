/*----------------------------------------------------------------------------*
 *                                                                            *
 *                              n w - s d p                                   * 
 *                    S o f t     D a t a     P l a n e                       *
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "NwTypes.h"
#include "NwUtils.h"
#include "NwSdpError.h"
#include "NwSdpPrivate.h"
#include "NwSdpFlowContext.h"
#include "NwSdp.h"
#include "NwSdpLog.h"

#include "NwGtpv1u.h"
#include "NwGtpv1uMsg.h"
#include "NwGre.h"
#include "NwGreMsg.h"
#include "NwIpv4.h"
#include "NwIpv4Msg.h"

#define NW_SDP_RESPOND_ICMP_PING        (1)

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------*
 *                    P R I V A T E    F U N C T I O N S                    *
 *--------------------------------------------------------------------------*/

#define MAX_UDP_PAYLOAD_LEN             (4096)

#ifdef NW_SDP_SUPPORT_UDP_FLOW_TYPE
static
void NW_EVT_CALLBACK(nwSdpUdpDataIndicationCallback)
{
  NwSdpFlowContextT* thiz = (NwSdpFlowContextT*) arg;
  NwSdpRcT         rc;
  NwU8T         udpBuf[MAX_UDP_PAYLOAD_LEN];
  NwS32T        bytesRead;
  NwU32T        peerLen;
  struct sockaddr_in peer;

  peerLen = sizeof(peer);

  bytesRead = recvfrom(thiz->hSocket, udpBuf, MAX_UDP_PAYLOAD_LEN , 0, (struct sockaddr *) &peer,(socklen_t*) &peerLen);
  if(bytesRead)
  {
    NW_LOG(thiz->pStack, NW_LOG_LEVEL_DEBG, "Received UDP message of length %u from %X:%u", bytesRead, ntohl(peer.sin_addr.s_addr), ntohs(peer.sin_port));


    switch(thiz->egressEndPoint.flowType)
    {
      case NW_FLOW_TYPE_GRE:
        {
          NwGreUlpApiT           ulpReq;

          /*
           * Send Message Request to GRE Stack Instance
           */

          ulpReq.apiType                        = NW_GRE_ULP_API_SEND_TPDU;
          ulpReq.apiInfo.sendtoInfo.teid        = thiz->egressEndPoint.flowKey.greKey;
          ulpReq.apiInfo.sendtoInfo.ipAddr      = thiz->egressEndPoint.ipv4Addr;

          rc = nwGreGpduMsgNew( thiz->pStack->hGreStack,
              NW_FALSE,
              NW_TRUE,
              NW_FALSE,
              thiz->egressEndPoint.flowKey.greKey,
              0,
              udpBuf,
              bytesRead,
              &(ulpReq.apiInfo.sendtoInfo.hMsg));

          NW_ASSERT( rc == NW_SDP_OK );

          rc = nwGreProcessUlpReq(thiz->pStack->hGreStack, &ulpReq);
          NW_ASSERT( rc == NW_SDP_OK );

          rc = nwGreMsgDelete(thiz->pStack->hGreStack, (ulpReq.apiInfo.sendtoInfo.hMsg));
          NW_ASSERT( rc == NW_SDP_OK );

        }
      case NW_FLOW_TYPE_GTPU:
        {
          NwGtpv1uUlpApiT           ulpReq;

          /*
           * Send Message Request to GRE Stack Instance
           */

          ulpReq.apiType                        = NW_GTPV1U_ULP_API_SEND_TPDU;
          ulpReq.apiInfo.sendtoInfo.teid        = thiz->egressEndPoint.flowKey.gtpuTeid;
          ulpReq.apiInfo.sendtoInfo.ipAddr      = thiz->egressEndPoint.ipv4Addr;

        }
        break;
      default:
        {
          NW_ASSERT(0);
        }
    }
  }
  else
  {
    NW_LOG(thiz->pStack, NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
  } 
}
#endif

static NwSdpRcT
nwSdpCreateFlowEndPoint( NW_IN  NwSdpT* thiz,  
    NW_IN NwSdpFlowContextT* pFlowContext,
    NW_IN NwSdpFlowEndPointT*  pFlowEndPoint)
{
  NwSdpRcT rc  = NW_SDP_OK;

  switch(pFlowEndPoint->flowType)
  {
    case NW_FLOW_TYPE_IPv4:
      {
        NwIpv4UlpApiT         ulpReq;
        NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Creating IPv4 tunnel endpoint with teid "NW_IPV4_ADDR, NW_IPV4_ADDR_FORMAT(pFlowEndPoint->flowKey.ipv4Addr));
        if(thiz->hIpv4Stack)
        {
          ulpReq.apiType                                        = NW_IPv4_ULP_API_CREATE_TUNNEL_ENDPOINT;
          ulpReq.apiInfo.createTunnelEndPointInfo.ipv4Addr      = pFlowEndPoint->flowKey.ipv4Addr;
          ulpReq.apiInfo.createTunnelEndPointInfo.hUlpSession   = (NwGtpv1uUlpSessionHandleT)pFlowContext;

          rc = nwIpv4ProcessUlpReq(thiz->hIpv4Stack, &ulpReq);

          pFlowEndPoint->hTunnelEndPoint.ipv4                   = ulpReq.apiInfo.createTunnelEndPointInfo.hStackSession;
        }
        else
        {
          NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "IPv4 service does not exist on data plane!");
          rc = NW_SDP_OK;
        }
      }
      break;

    case NW_FLOW_TYPE_GTPU:
      {
        NwGtpv1uUlpApiT         ulpReq;
        NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Creating GTPU tunnel endpoint with teid 0x%x", pFlowEndPoint->flowKey.gtpuTeid);
        if(thiz->hGtpv1uStack)
        {
          ulpReq.apiType                                        = NW_GTPV1U_ULP_API_CREATE_TUNNEL_ENDPOINT;
          ulpReq.apiInfo.createTunnelEndPointInfo.teid          = pFlowEndPoint->flowKey.gtpuTeid;
          ulpReq.apiInfo.createTunnelEndPointInfo.hUlpSession   = (NwGtpv1uUlpSessionHandleT)pFlowContext;

          rc = nwGtpv1uProcessUlpReq(thiz->hGtpv1uStack, &ulpReq);

          pFlowEndPoint->hTunnelEndPoint.gtpu                     = ulpReq.apiInfo.createTunnelEndPointInfo.hStackSession;
        }
        else
        {
          NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "GTPU service does not exist on data plane!");
          rc = NW_SDP_OK;
        }
      }
      break;

    case NW_FLOW_TYPE_GRE:
      {
        NwGreUlpApiT            ulpReq;
        NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Creating GRE tunnel endpoint with key %d", pFlowEndPoint->flowKey.greKey);
        if(thiz->hGreStack)
        {
          ulpReq.apiType                                        = NW_GRE_ULP_API_CREATE_TUNNEL_ENDPOINT;
          ulpReq.apiInfo.createTunnelEndPointInfo.greKey        = pFlowEndPoint->flowKey.greKey;
          ulpReq.apiInfo.createTunnelEndPointInfo.hUlpSession   = (NwGtpv1uUlpSessionHandleT)pFlowContext;

          rc = nwGreProcessUlpReq(thiz->hGreStack, &ulpReq);
          pFlowEndPoint->hTunnelEndPoint.gre                    = ulpReq.apiInfo.createTunnelEndPointInfo.hStackSession;
        }
        else
        {
          NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "GRE service does not exist on data plane!");
          rc = NW_SDP_OK;
        }
      }
      break;

    case NW_FLOW_TYPE_UDP:
      {
#ifdef NW_SDP_SUPPORT_UDP_FLOW_TYPE
        /*
         * Create local udp listening endpoint
         */

        struct sockaddr_in addr;

        int sd = socket(AF_INET, SOCK_DGRAM, 0);

        if (sd < 0)
        {
          NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
          NW_ASSERT(0);
        }

        addr.sin_family       = AF_INET;
        addr.sin_port         = htons(pFlowEndPoint->flowKey.udp.port);
        addr.sin_addr.s_addr  = pFlowEndPoint->flowKey.udp.ipv4Addr;
        memset(addr.sin_zero, '\0', sizeof (addr.sin_zero));

        if(bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
          NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "bind - %s", strerror(errno));
        }

        NW_LOG(thiz, NW_LOG_LEVEL_NOTI, "Creating UDP tunnel endpoint with port %d", pFlowEndPoint->flowKey.udp.port);

        event_set(&(pFlowContext->ev), sd, EV_READ|EV_PERSIST, nwSdpUdpDataIndicationCallback, pFlowContext);
        event_add(&(pFlowContext->ev), NULL);

        pFlowEndPoint->hTunnelEndPoint.udp = sd;
#else
        NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "Flow type UDP not supported");
#endif
      }
      break;

    default:
      {
        NW_LOG(thiz, NW_LOG_LEVEL_NOTI, "Unsupported encapsulation type %u", pFlowEndPoint->flowType);
        return NW_SDP_FAILURE;
      }
  }

  return NW_SDP_OK;
}

static NwSdpRcT
nwSdpDestroyFlowEndPoint( NW_IN  NwSdpT* thiz,  
    NW_IN NwSdpFlowContextT* pFlowContext,
    NW_IN NwSdpFlowEndPointT* pFlowEndPoint)
{
  NwSdpRcT rc  = NW_SDP_OK;

  switch(pFlowEndPoint->flowType)
  {
    case NW_FLOW_TYPE_IPv4:
      {
        NwIpv4UlpApiT         ulpReq;
        NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Destroying IPv4 tunnel endpoint with teid 0x%x", pFlowEndPoint->flowKey.ipv4Addr);
        if(thiz->hIpv4Stack && pFlowEndPoint->hTunnelEndPoint.ipv4)
        {
          ulpReq.apiType                                        = NW_IPv4_ULP_API_DESTROY_TUNNEL_ENDPOINT;
          ulpReq.apiInfo.destroyTunnelEndPointInfo.hStackSessionHandle = (NwIpv4UlpSessionHandleT)pFlowEndPoint->hTunnelEndPoint.ipv4;
          rc = nwIpv4ProcessUlpReq(thiz->hIpv4Stack, &ulpReq);
        }
        else
        {
          NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "IPv4 end point does not exist on data plane!");
          rc = NW_SDP_OK;
        }
      }
      break;

    case NW_FLOW_TYPE_GTPU:
      {
        NwGtpv1uUlpApiT         ulpReq;
        NW_LOG(thiz, NW_LOG_LEVEL_INFO, "Destroying GTPU tunnel endpoint with teid 0x%x for handle 0x%x", pFlowEndPoint->flowKey.gtpuTeid, (NwGtpv1uUlpSessionHandleT)pFlowEndPoint->hTunnelEndPoint.gtpu);
        if(thiz->hGtpv1uStack && pFlowEndPoint->hTunnelEndPoint.gtpu)
        {
          ulpReq.apiType                                        = NW_GTPV1U_ULP_API_DESTROY_TUNNEL_ENDPOINT;
          ulpReq.apiInfo.destroyTunnelEndPointInfo.hStackSessionHandle = (NwGtpv1uUlpSessionHandleT)pFlowEndPoint->hTunnelEndPoint.gtpu;
          rc = nwGtpv1uProcessUlpReq(thiz->hGtpv1uStack, &ulpReq);
        }
        else
        {
          NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "GTPU tunnel end point does not exist on data plane!");
          rc = NW_SDP_OK;
        }
      }
      break;

    case NW_FLOW_TYPE_GRE:
      {
        NwGreUlpApiT            ulpReq;
        NW_LOG(thiz, NW_LOG_LEVEL_NOTI, "Destroying GRE tunnel endpoint with key %d", pFlowEndPoint->flowKey.greKey);
        if(thiz->hGreStack && pFlowEndPoint->hTunnelEndPoint.gre)
        {
          ulpReq.apiType                                        = NW_GRE_ULP_API_DESTROY_TUNNEL_ENDPOINT;
          ulpReq.apiInfo.destroyTunnelEndPointInfo.hStackSessionHandle = (NwGtpv1uUlpSessionHandleT)pFlowEndPoint->hTunnelEndPoint.gre;

          rc = nwGreProcessUlpReq(thiz->hGreStack, &ulpReq);
        }
        else
        {
          NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "GRE tunnel end point does not exist on data plane!");
          rc = NW_SDP_OK;
        }
      }
      break;

    case NW_FLOW_TYPE_UDP:
      {
#ifdef NW_SDP_SUPPORT_UDP_FLOW_TYPE
        /*
         * Destroy local udp listening endpoint
         */

        NW_LOG(thiz, NW_LOG_LEVEL_NOTI, "Destroying UDP tunnel endpoint with port %d", pFlowEndPoint->flowKey.udp.port);

        event_del(&(pFlowContext->ev));
        close(pFlowEndPoint->hTunnelEndPoint.gtpu);
#else
        NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "Flow type UDP not supported");
#endif
      }
      break;

    default:
      {
        NW_LOG(thiz, NW_LOG_LEVEL_NOTI, "Unsupported encapsulation type %u", pFlowEndPoint->flowType);
        return NW_SDP_FAILURE;
      }
  }

  return NW_SDP_OK;
}

/*---------------------------------------------------------------------------
 * ULP API Processing Functions 
 *--------------------------------------------------------------------------*/

/**
  Process NW_SDP_ULP_API_CREATE_FLOW Request from ULP entity.

  @param[in] hSdp : Stack handle
  @param[in] pUlpReq : Pointer to Ulp Req.
  @return NW_SDP_OK on success.
 */

static NwSdpRcT
nwSdpCreateFlow( NW_IN  NwSdpT* thiz,  
    NW_IN NwSdpUlpApiT *pUlpReq)
{
  NwSdpRcT rc = NW_SDP_OK;
  NwSdpFlowContextT* pFlowContext;
  NwSdpCreateFlowInfoT* pCreateFlowInfo;
  NW_ENTER(thiz);

  pFlowContext = nwSdpFlowContextNew(thiz);

  if(pFlowContext)
  {
    pCreateFlowInfo = &(pUlpReq->apiInfo.createFlowInfo);

    pFlowContext->ingressEndPoint = pCreateFlowInfo->ingressEndPoint;
    pFlowContext->egressEndPoint  = pCreateFlowInfo->egressEndPoint;

    if(pFlowContext->egressEndPoint.flowType == NW_FLOW_TYPE_GTPU)
    {
      NW_ASSERT(pFlowContext->egressEndPoint.ipv4Addr != 0);
    }
    rc = nwSdpCreateFlowEndPoint(thiz, pFlowContext, &pFlowContext->ingressEndPoint);
    NW_ASSERT(rc == NW_SDP_OK);

    pFlowContext->pStack = thiz;
  }
  else
  {
    NW_ASSERT(pFlowContext);
  }
  
  pUlpReq->apiInfo.createFlowInfo.hSdpSession = (NwSdpSessionHandleT) pFlowContext;

  NW_LEAVE(thiz);
  return rc;
}

/**
  Process NW_SDP_ULP_API_DESTROY_FLOW Request from ULP entity.

  @param[in] hSdp : Stack handle
  @param[in] pUlpReq : Pointer to Ulp Req.
  @return NW_SDP_OK on success.
 */

static NwSdpRcT
nwSdpDestroyFlow( NwSdpT* thiz,  NW_IN NwSdpUlpApiT *pUlpReq)
{
  NwSdpRcT rc = NW_SDP_OK;
  NwSdpFlowContextT* pFlowContext = (NwSdpFlowContextT*)pUlpReq->apiInfo.destroyFlowInfo.hSdpSession;

  rc = nwSdpDestroyFlowEndPoint(thiz, pFlowContext, &pFlowContext->ingressEndPoint);
  NW_ASSERT(rc == NW_SDP_OK);

  rc = nwSdpFlowContextDelete(thiz, pFlowContext);
  NW_ASSERT(rc == NW_SDP_OK);

  return rc;
}

/**
  Process NW_SDP_ULP_API_UPDATE_FLOW Request from ULP entity.

  @param[in] hSdp : Stack handle
  @param[in] pUlpReq : Pointer to Ulp Req.
  @return NW_SDP_OK on success.
 */

static NwSdpRcT
nwSdpUpdateFlow( NW_IN NwSdpT* thiz, NW_IN NwSdpUlpApiT *pUlpReq)
{
  NwSdpRcT rc;

  NW_ENTER(thiz);
  NW_LEAVE(thiz);

  return rc;
}

NwSdpRcT
nwSdpProcessIpv4DataIndication(NwSdpT* thiz, 
              NwSdpFlowContextT* pFlowContext,
              NwIpv4MsgHandleT hMsg)
{
  NwSdpRcT rc;

  /*
   * Send Message Request to GTPv1u Stack Instance
   */

  switch(pFlowContext->egressEndPoint.flowType)
  {
    case NW_FLOW_TYPE_GTPU:
      {
        NwGtpv1uUlpApiT           ulpReq;
        NW_ASSERT(pFlowContext->egressEndPoint.ipv4Addr != 0);
        NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Sending IP PDU over GTPU teid 0x%x to "NW_IPV4_ADDR, pFlowContext->egressEndPoint.flowKey.gtpuTeid, NW_IPV4_ADDR_FORMAT(htonl(pFlowContext->egressEndPoint.ipv4Addr)));
        ulpReq.apiType                        = NW_GTPV1U_ULP_API_SEND_TPDU;
        ulpReq.apiInfo.sendtoInfo.teid        = pFlowContext->egressEndPoint.flowKey.gtpuTeid;
        ulpReq.apiInfo.sendtoInfo.ipAddr      = pFlowContext->egressEndPoint.ipv4Addr;

        rc = nwGtpv1uGpduMsgNew( thiz->hGtpv1uStack,
            pFlowContext->egressEndPoint.flowKey.gtpuTeid,      /* TEID                 */
            NW_FALSE,                                           /* Seq Num Present Flag */
            0,                                                  /* seqNum               */
            (NwU8T*) nwIpv4MsgGetBufHandle(thiz->hIpv4Stack, hMsg),
            nwIpv4MsgGetLength(thiz->hIpv4Stack, hMsg),
            (NwGtpv1uMsgHandleT*)&(ulpReq.apiInfo.sendtoInfo.hMsg));

        NW_ASSERT( rc == NW_SDP_OK );

        rc = nwGtpv1uProcessUlpReq(thiz->hGtpv1uStack, &ulpReq);
        NW_ASSERT( rc == NW_SDP_OK );

        rc = nwGtpv1uMsgDelete(thiz->hGtpv1uStack, (ulpReq.apiInfo.sendtoInfo.hMsg));
        NW_ASSERT( rc == NW_SDP_OK );
      }
      break;
    case NW_FLOW_TYPE_GRE:
      {
        NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Cannot send IP PDU over GRE! Not supported yet!");
      }
      break;

    default:
      {
        NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "Unsupported egress flow end point type! Dropping IP PDU.");
      }
      break;
  }

  return NW_SDP_OK;
}

NwSdpRcT
nwSdpProcessGtpuDataIndication(NwSdpT* thiz, 
              NwSdpFlowContextT* pFlowContext,
              NwGtpv1uMsgHandleT hMsg)
{
  NwSdpRcT rc;

  /*
   * Send Message Request to GTPv1u Stack Instance
   */

  switch(pFlowContext->egressEndPoint.flowType)
  {
    case NW_FLOW_TYPE_IPv4:
      {
        NwIpv4UlpApiT           ulpReq;
        NwU8T*                  pIpv4Pdu;

        if(thiz->hIpv4Stack)
        {
          /* Send over IP*/
          rc = nwIpv4MsgFromBufferNew(thiz->hIpv4Stack,  
              nwGtpv1uMsgGetTpduHandle(hMsg),
              nwGtpv1uMsgGetTpduLength(hMsg),
              &(ulpReq.apiInfo.sendtoInfo.hMsg));
          NW_ASSERT(NW_OK == rc);

#ifdef NW_SDP_RESPOND_ICMP_PING
          pIpv4Pdu = nwIpv4MsgGetBufHandle(thiz->hIpv4Stack, ulpReq.apiInfo.sendtoInfo.hMsg);
          if(*(pIpv4Pdu + 20) == 0x08)
          {
#define NW_MAX_ICMP_PING_DATA_SIZE      (1024)
            NwU8T pingRspPdu[14 + NW_MAX_ICMP_PING_DATA_SIZE];
            NwU32T pingRspPduLen;

            *(pIpv4Pdu + 20) = 0x00;

            pingRspPdu[12] = 0x08;
            pingRspPdu[13] = 0x00;

            pingRspPduLen = (NW_MAX_ICMP_PING_DATA_SIZE > nwIpv4MsgGetLength(thiz->hIpv4Stack, ulpReq.apiInfo.sendtoInfo.hMsg) ? nwIpv4MsgGetLength(thiz->hIpv4Stack, ulpReq.apiInfo.sendtoInfo.hMsg) : NW_MAX_ICMP_PING_DATA_SIZE);
            memcpy(pingRspPdu + 14, pIpv4Pdu, pingRspPduLen);
            memcpy(pingRspPdu + 14 + 16, pIpv4Pdu + 12, 4);
            memcpy(pingRspPdu + 14 + 12, pIpv4Pdu + 16, 4);
            /* TODO: Add ip-checksum */
            rc = nwSdpProcessIpv4DataInd(thiz, 0, pingRspPdu, pingRspPduLen + 14);
          }
          else
          {
#endif
            ulpReq.apiType          = NW_IPv4_ULP_API_SEND_TPDU;
            rc = nwIpv4ProcessUlpReq(thiz->hIpv4Stack, &ulpReq);
            NW_ASSERT( rc == NW_SDP_OK );
#ifdef NW_SDP_RESPOND_ICMP_PING
          }
#endif
          rc = nwIpv4MsgDelete(thiz->hIpv4Stack, (ulpReq.apiInfo.sendtoInfo.hMsg));
          NW_ASSERT( rc == NW_SDP_OK );

        }
        else
        {
          NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "Cannot send PDU over IPv4! IPv4 service does not exist on data plane.");
        }
      }
      break;

    case NW_FLOW_TYPE_GTPU:
      {
        NwGtpv1uUlpApiT           ulpReq;
        ulpReq.apiType                        = NW_GTPV1U_ULP_API_SEND_TPDU;
        ulpReq.apiInfo.sendtoInfo.teid        = pFlowContext->egressEndPoint.flowKey.gtpuTeid;
        ulpReq.apiInfo.sendtoInfo.ipAddr      = pFlowContext->egressEndPoint.ipv4Addr;

        if(thiz->hGtpv1uStack)
        {
          rc = nwGtpv1uMsgFromMsgNew( thiz->hGtpv1uStack,
              hMsg,
              &(ulpReq.apiInfo.sendtoInfo.hMsg));

          NW_ASSERT( rc == NW_SDP_OK );

          rc = nwGtpv1uProcessUlpReq(thiz->hGtpv1uStack, &ulpReq);
          NW_ASSERT( rc == NW_SDP_OK );

          rc = nwGtpv1uMsgDelete(thiz->hGtpv1uStack, (ulpReq.apiInfo.sendtoInfo.hMsg));
          NW_ASSERT( rc == NW_SDP_OK );
          NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Sending GTPU PDU over teid 0x%x "NW_IPV4_ADDR, ulpReq.apiInfo.sendtoInfo.teid, NW_IPV4_ADDR_FORMAT(ntohl(ulpReq.apiInfo.sendtoInfo.ipAddr)));
        }
        else
        {
          NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "Cannot send PDU over GTPU! GTPU service does not exist on data plane.");
        }
      }
      break;
    case NW_FLOW_TYPE_GRE:
      {
        NW_LOG(thiz, NW_LOG_LEVEL_WARN, "Cannot send TPDU over GRE! Not supported yet!");
      }
      break;
    default:
      {
        NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "Unsupported egress flow end point type! Dropping GTP TPDU.");
      }
      break;
  }

  return NW_SDP_OK;
}

static
NwSdpRcT nwSdpProcessGtpv1uStackReqCallback(NwGtpv1uUlpHandleT hUlp,
                           NwGtpv1uUlpApiT *pUlpApi)
{
  NwSdpRcT rc;
  NwSdpT* thiz = (NwSdpT*)hUlp;

  switch(pUlpApi->apiType)
  {
    case NW_GTPV1U_ULP_API_RECV_TPDU:
      {
        NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Received T-PDU from GTPU Stack");
        rc = nwSdpProcessGtpuDataIndication(thiz, (NwSdpFlowContextT*)pUlpApi->apiInfo.recvMsgInfo.hUlpSession, pUlpApi->apiInfo.recvMsgInfo.hMsg);
      }
      break;

    default:
      NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "Received Unhandled API request from GTPU Stack");
      break;
  }

  rc = nwGtpv1uMsgDelete(thiz->hGtpv1uStack,  pUlpApi->apiInfo.recvMsgInfo.hMsg);
  NW_ASSERT( rc == NW_SDP_OK );

  return NW_SDP_OK;
}

static NwRcT 
nwSdpProcessGreStackReqCallback(NwGreUlpHandleT hUlp,
                           NwGreUlpApiT *pUlpApi)
{
  return NW_SDP_OK;
}

static
NwSdpRcT nwSdpProcessIpv4StackReqCallback(NwGreUlpHandleT hUlp,
                           NwIpv4UlpApiT *pUlpApi)
{
  NwSdpRcT rc;
  NwSdpT* thiz = (NwSdpT*)hUlp;
  NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Received API request from IPv4 Stack for ULP session 0x%x", pUlpApi->apiInfo.recvMsgInfo.hUlpSession);

  switch(pUlpApi->apiType)
  {
    case NW_IPv4_ULP_API_RECV_TPDU:
      {
        NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Received T-PDU from IPv4 Stack");
        rc = nwSdpProcessIpv4DataIndication(thiz, (NwSdpFlowContextT*)pUlpApi->apiInfo.recvMsgInfo.hUlpSession, pUlpApi->apiInfo.recvMsgInfo.hMsg);
      }
      break;

    default:
      NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "Received Unhandled API request from IPv4 Stack");
      break;
  }

  rc = nwIpv4MsgDelete(thiz->hIpv4Stack,  pUlpApi->apiInfo.recvMsgInfo.hMsg);
  NW_ASSERT( rc == NW_SDP_OK );

  return NW_SDP_OK;

  return NW_SDP_OK;
}

/*--------------------------------------------------------------------------*
 *                     P U B L I C   F U N C T I O N S                      *
 *--------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * Constructor
 *--------------------------------------------------------------------------*/

NwSdpRcT
nwSdpInitialize( NW_INOUT NwSdpHandleT* hSdp)
{
  NwSdpRcT rc;
  NwSdpT* thiz;

  thiz = (NwSdpT*) malloc ( sizeof(NwSdpT));

  if(thiz)
  {
    thiz->id    = (NwU32T) thiz;
    thiz->seq   = (NwU16T) ((NwU32T)thiz) ;
    rc = NW_SDP_OK;
  }
  else
  {
    rc = NW_SDP_FAILURE;
  }


  *hSdp = (NwSdpHandleT) thiz;
  return rc;
}


/*---------------------------------------------------------------------------
 * Destructor
 *--------------------------------------------------------------------------*/

NwSdpRcT
nwSdpFinalize( NW_IN  NwSdpHandleT hSdp)
{
  NwSdpRcT rc;
  if(hSdp)
  {
    free((void*)hSdp);
    rc = NW_SDP_OK;
  }
  else
  {
    rc = NW_SDP_FAILURE;
  }
  return rc;
}


/** 
  Create a new SDP GRE service instance

 @param[in] hSdp : Stack handle.
 @param[in] hTlService: Transport Layer Service handle.
 @param[in] pTlDataReqCb: Transport Layer Service data request callback.
 @param[in] serviceCfg: Service configuration.
 @param[out] phSdpService: Pointer to SDP Service handle.
 @return NW_SDP_OK on success.
 */

NwSdpRcT
nwSdpCreateGreService1( NW_IN NwSdpHandleT hSdp,
                       NW_IN NwSdpServiceHandleT* phSdpService)
{
  NwSdpRcT rc;
  NwSdpT* thiz = (NwSdpT*) hSdp;

  NwGreStackHandleT             hGreStack;
  NwGreTimerMgrEntityT          greTmrMgr;
  NwGreUlpEntityT               greUlp;
  /*
   * Create GRE Stack Instance
   */

  rc = nwGreInitialize(&hGreStack);
  if(rc != NW_SDP_OK)
  {
    NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "Failed to create SDP instance. Error '%u' occured", rc);
    exit(1);
  }
  NW_LOG(thiz, NW_LOG_LEVEL_INFO, "Gre Stack Handle '%X' Creation Successful!", hGreStack);

  greUlp.hUlp                   = (NwGreUlpHandleT) thiz;
  greUlp.ulpReqCallback         = nwSdpProcessGreStackReqCallback;

  rc = nwGreSetUlpEntity(hGreStack, &greUlp);
  NW_ASSERT( rc == NW_SDP_OK );

  greTmrMgr.tmrMgrHandle        = thiz->tmrMgr.tmrMgrHandle;
  greTmrMgr.tmrStartCallback    = thiz->tmrMgr.tmrStartCallback;
  greTmrMgr.tmrStopCallback     = thiz->tmrMgr.tmrStopCallback;

  rc = nwGreSetTimerMgrEntity(thiz->hGreStack, &greTmrMgr);
  NW_ASSERT( rc == NW_SDP_OK );

  *phSdpService = (NwSdpServiceHandleT) hGreStack;

  return rc;
}

/** 
 Create Ipv4 service instance

 @param[in] hSdp : Stack handle.
 @param[in] mode : Uplink or downlink mode.
 @param[in] hTlService: Transport Layer Service handle.
 @param[in] pTlDataReqCb: Transport Layer Service data request callback.
 @param[in] serviceCfg: Service configuration.
 @param[out] phSdpService: Pointer to SDP Service handle.
 @return NW_SDP_OK on success.
 */

NwSdpRcT
nwSdpCreateIpv4Service( NW_IN NwSdpHandleT      hSdp,
                        NW_IN NwU32T            mode,
                        NW_IN NwU8T             *pHwAddr,
                        NW_IN NwU32T            hIpv4TlInterface,
                        NW_IN NwSdpRcT          (*pIpv4TlDataReqCb)( NwU32T udpHandle,
                                                        NwU8T* dataBuf,
                                                        NwU32T dataSize),
                        NW_IN NwSdpRcT          (*pIpv4TlArpDataReqCb) (
                                                        NwU32T          udpHandle,       
                                                        NwU16T           opCode,
                                                        NwU8T            *pTargetMac,
                                                        NwU8T            *pTargetIpAddr,
                                                        NwU8T            *pSenderIpAddr),


                       NW_IN NwSdpServiceHandleT* phSdpService)
{
  NwSdpRcT rc = NW_SDP_OK;
  NwSdpT* thiz;
  NwIpv4UlpEntityT         ipv4Ulp;
  NwIpv4LlpEntityT         ipv4Llp;
  NwIpv4LogMgrEntityT      ipv4LogMgr;
  NwIpv4MemMgrEntityT      ipv4MemMgr;
  NwIpv4TimerMgrEntityT    ipv4TmrMgr;

  thiz = (NwSdpT*) hSdp;

  /*
   * Create IP Stack Instance
   */

  rc = nwIpv4Initialize(&thiz->hIpv4Stack);
  if(rc != NW_SDP_OK)
  {
    NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "Failed to create IPv4 stack instance. Error '%u' occured", rc);
    exit(1);
  }
  NW_LOG(thiz, NW_LOG_LEVEL_INFO, "IPv4 Stack Handle '%X' Creation Successful!", thiz->hIpv4Stack);

  /*---------------------------------------------------------------------------
   * Set up mode uplink or downlink. 
   *--------------------------------------------------------------------------*/
  rc = nwIpv4SetMode(thiz->hIpv4Stack, mode);
  NW_ASSERT( rc == NW_SDP_OK );

  /*---------------------------------------------------------------------------
   * Set up Mem Entity 
   *--------------------------------------------------------------------------*/

  ipv4MemMgr.hMemMgr            = thiz->memMgr.hMemMgr;
  ipv4MemMgr.memAlloc           = thiz->memMgr.memAlloc;
  ipv4MemMgr.memFree            = thiz->memMgr.memFree;

  rc = nwIpv4SetMemMgrEntity(thiz->hIpv4Stack, &ipv4MemMgr);
  NW_ASSERT( rc == NW_SDP_OK );


  /*---------------------------------------------------------------------------
   * Set up Log Entity 
   *--------------------------------------------------------------------------*/

  ipv4TmrMgr.tmrMgrHandle        = thiz->tmrMgr.tmrMgrHandle;
  ipv4TmrMgr.tmrStartCallback    = thiz->tmrMgr.tmrStartCallback;
  ipv4TmrMgr.tmrStopCallback     = thiz->tmrMgr.tmrStopCallback;

  rc = nwIpv4SetTimerMgrEntity(thiz->hIpv4Stack, &ipv4TmrMgr);
  NW_ASSERT( rc == NW_SDP_OK );

  /*---------------------------------------------------------------------------
   * Set up Log Entity 
   *--------------------------------------------------------------------------*/

  ipv4LogMgr.logMgrHandle   = (NwIpv4LogMgrHandleT) thiz->logMgr.logMgrHandle;
  ipv4LogMgr.logReqCallback = thiz->logMgr.logReqCallback;

  rc = nwIpv4SetLogMgrEntity(thiz->hIpv4Stack, &ipv4LogMgr);
  NW_ASSERT( rc == NW_SDP_OK );

  /*---------------------------------------------------------------------------
   * Set Ipv4 log level  
   *--------------------------------------------------------------------------*/

  rc = nwIpv4SetLogLevel(thiz->hIpv4Stack, thiz->logLevel);

  /*---------------------------------------------------------------------------
   * Set ULP Entity 
   *--------------------------------------------------------------------------*/

  ipv4Ulp.hUlp = (NwIpv4UlpHandleT) thiz;
  ipv4Ulp.ulpReqCallback = nwSdpProcessIpv4StackReqCallback;

  rc = nwIpv4SetUlpEntity(thiz->hIpv4Stack, &ipv4Ulp);
  NW_ASSERT( rc == NW_SDP_OK );

  /*---------------------------------------------------------------------------
   * Set LLP Entity 
   *--------------------------------------------------------------------------*/

  memcpy(ipv4Llp.llpHwAddr, pHwAddr, 6);
  ipv4Llp.hLlp = (NwIpv4LlpHandleT) hIpv4TlInterface;
  ipv4Llp.llpDataReqCallback    = pIpv4TlDataReqCb;
  ipv4Llp.llpArpDataReqCallback = pIpv4TlArpDataReqCb;

  rc = nwIpv4SetLlpEntity(thiz->hIpv4Stack, &ipv4Llp);
  NW_ASSERT( rc == NW_SDP_OK );


  *phSdpService = (NwSdpServiceHandleT) (thiz->hIpv4Stack);

  return rc;
}

/** 
 Create GTPU service instance

 @param[in] hSdp : Stack handle.
 @param[in] hTlService: Transport Layer Service handle.
 @param[in] pTlDataReqCb: Transport Layer Service data request callback.
 @param[in] serviceCfg: Service configuration.
 @param[out] phSdpService: Pointer to SDP Service handle.
 @return NW_SDP_OK on success.
 */

NwSdpRcT
nwSdpCreateGtpuService( NW_IN NwSdpHandleT hSdp,
                        NW_IN NwU32T hGtpuTlInterface,
                        NW_IN NwSdpRcT (*pGtpuTlDataReqCb)( NwGtpv1uUdpHandleT hUdp,
                                                        NwU8T* dataBuf,
                                                        NwU32T dataSize,
                                                        NwU32T peerIpAddr,
                                                        NwU32T peerPort),
                       NW_IN NwSdpServiceHandleT* phSdpService)
{
  NwSdpRcT rc;
  NwSdpT* thiz;
  NwGtpv1uUlpEntityT         gtpuUlp;
  NwGtpv1uUdpEntityT         gtpuUdp;
  NwGtpv1uLogMgrEntityT      gtpuLogMgr;
  NwGtpv1uTimerMgrEntityT    gtpuTmrMgr;
  NwGtpv1uMemMgrEntityT      gtpuMemMgr;

  thiz = (NwSdpT*) hSdp;

  /*
   * Create GTPU Stack Instance
   */

  rc = nwGtpv1uInitialize(&thiz->hGtpv1uStack);
  if(rc != NW_SDP_OK)
  {
    NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "Failed to create GTPU stack instance. Error '%u' occured", rc);
    exit(1);
  }
  NW_LOG(thiz, NW_LOG_LEVEL_INFO, "GTPU Stack Handle '%X' Creation Successful!", thiz->hGtpv1uStack);

  /*---------------------------------------------------------------------------
   * Set up Mem Manager 
   *--------------------------------------------------------------------------*/

  gtpuMemMgr.hMemMgr            = thiz->memMgr.hMemMgr;
  gtpuMemMgr.memAlloc           = thiz->memMgr.memAlloc;
  gtpuMemMgr.memFree            = thiz->memMgr.memFree;

  rc = nwGtpv1uSetMemMgrEntity(thiz->hGtpv1uStack, &gtpuMemMgr);
  NW_ASSERT( rc == NW_SDP_OK );


  /*---------------------------------------------------------------------------
   * Set up Tmr Manager 
   *--------------------------------------------------------------------------*/

  gtpuTmrMgr.tmrMgrHandle        = thiz->tmrMgr.tmrMgrHandle;
  gtpuTmrMgr.tmrStartCallback    = thiz->tmrMgr.tmrStartCallback;
  gtpuTmrMgr.tmrStopCallback     = thiz->tmrMgr.tmrStopCallback;

  rc = nwGtpv1uSetTimerMgrEntity(thiz->hGtpv1uStack, &gtpuTmrMgr);
  NW_ASSERT( rc == NW_SDP_OK );

  /*---------------------------------------------------------------------------
   * Set up Log Entity 
   *--------------------------------------------------------------------------*/

  gtpuLogMgr.logMgrHandle   = (NwGtpv1uLogMgrHandleT) thiz->logMgr.logMgrHandle;
  gtpuLogMgr.logReqCallback = thiz->logMgr.logReqCallback;

  rc = nwGtpv1uSetLogMgrEntity(thiz->hGtpv1uStack, &gtpuLogMgr);
  NW_ASSERT( rc == NW_SDP_OK );

  /*---------------------------------------------------------------------------
   * Set GTPv1u log level  
   *--------------------------------------------------------------------------*/

  rc = nwGtpv1uSetLogLevel(thiz->hGtpv1uStack, thiz->logLevel);

  /*---------------------------------------------------------------------------
   * Set ULP Entity 
   *--------------------------------------------------------------------------*/

  gtpuUlp.hUlp = (NwGtpv1uUlpHandleT) thiz;
  gtpuUlp.ulpReqCallback = nwSdpProcessGtpv1uStackReqCallback;

  rc = nwGtpv1uSetUlpEntity(thiz->hGtpv1uStack, &gtpuUlp);
  NW_ASSERT( rc == NW_SDP_OK );

  /*---------------------------------------------------------------------------
   * Set UDP Entity 
   *--------------------------------------------------------------------------*/

  gtpuUdp.hUdp = (NwGtpv1uUdpHandleT) hGtpuTlInterface;
  gtpuUdp.udpDataReqCallback = pGtpuTlDataReqCb;

  rc = nwGtpv1uSetUdpEntity(thiz->hGtpv1uStack, &gtpuUdp);
  NW_ASSERT( rc == NW_SDP_OK );

  *phSdpService = (NwSdpServiceHandleT) (thiz->hGtpv1uStack);

  return rc;
}

/** 
 Configure GRE service instance

 @param[in] hSdp : Stack handle.
 @param[in] hTlService: Transport Layer Service handle.
 @param[in] pTlDataReqCb: Transport Layer Service data request callback.
 @param[in] serviceCfg: Service configuration.
 @param[out] phSdpService: Pointer to SDP Service handle.
 @return NW_SDP_OK on success.
 */

NwSdpRcT
nwSdpCreateGreService(  NW_IN NwSdpHandleT hSdp,
                        NW_IN NwU32T hGreTlInterface,
                        NW_IN NwSdpRcT (*pGreTlDataReqCb)( NwU32T udpHandle,
                                                        NwU8T* dataBuf,
                                                        NwU32T dataSize,
                                                        NwU32T peerIpAddr,
                                                        NwU32T peerPort),
                       NW_IN NwSdpServiceHandleT* phSdpService)
{
  NwSdpRcT rc;
  NwSdpT* thiz = (NwSdpT*) hSdp;

  NwGreStackHandleT     hGreStack;
  NwGreUlpEntityT       greUlp;
  NwGreLogMgrEntityT    greLogMgr;
  NwGreTimerMgrEntityT  greTmrMgr;

  /* Create GRE Stack Instance */

  rc = nwGreInitialize(&hGreStack);
  if(rc != NW_SDP_OK)
  {
    NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "Failed to create SDP instance. Error '%u' occured", rc);
    exit(1);
  }
  NW_LOG(thiz, NW_LOG_LEVEL_INFO, "Gre Stack Handle '%X' Creation Successful!", hGreStack);

  greUlp.hUlp                   = (NwGreUlpHandleT) thiz;
  greUlp.ulpReqCallback         = nwSdpProcessGreStackReqCallback;

  rc = nwGreSetUlpEntity(hGreStack, &greUlp);
  NW_ASSERT( rc == NW_SDP_OK );

  greLogMgr.logMgrHandle        = (NwGreLogMgrHandleT) thiz->logMgr.logMgrHandle;
  greLogMgr.logReqCallback      = thiz->logMgr.logReqCallback;

  rc = nwGreSetLogMgrEntity(hGreStack, &greLogMgr);
  NW_ASSERT( rc == NW_SDP_OK );

  greTmrMgr.tmrMgrHandle        = thiz->tmrMgr.tmrMgrHandle;
  greTmrMgr.tmrStartCallback    = thiz->tmrMgr.tmrStartCallback;
  greTmrMgr.tmrStopCallback     = thiz->tmrMgr.tmrStopCallback;

  rc = nwGreSetTimerMgrEntity(hGreStack, &greTmrMgr);
  NW_ASSERT( rc == NW_SDP_OK );

  *phSdpService = (NwSdpServiceHandleT) hGreStack;

  return rc;
}

NwSdpRcT
nwSdpSetGreServiceTransportInterface( NW_IN NwSdpHandleT hSdp,
    NW_IN NwSdpServiceHandleT hSdpService,
    NW_IN NwU32T hGreTlInterface,
    NW_IN NwSdpRcT (*pGreTlDataReqCb)(NwU32T udpHandle,
      NwU8T* dataBuf,
      NwU32T dataSize,
      NwU32T peerIpAddr,
      NwU32T peerPort))
{
  NwSdpRcT rc;
  NwGreLlpEntityT greLlp;
  NwGreStackHandleT hGreStack = (NwGreStackHandleT) hSdpService;
  NwSdpT* thiz = (NwSdpT*) hSdp;

  greLlp.hUdp = (NwGreUdpHandleT) hGreTlInterface;
  greLlp.udpDataReqCallback = pGreTlDataReqCb;

  rc = nwGreSetUdpEntity(hGreStack, &greLlp);
  NW_ASSERT( rc == NW_SDP_OK );

  return rc;
}

/** 
  Create a new SDP service instance

 @param[in] hSdp : Stack handle.
 @param[out] phSdpService: Pointer to SDP Service handle.
 @return NW_SDP_OK on success.
 */

NwSdpRcT
nwSdpDestroyService( NW_IN NwSdpHandleT hSdp,
                    NW_IN NwSdpServiceHandleT* phSdpService)
{
  return NW_SDP_OK;
}


/*---------------------------------------------------------------------------
 * Configuration Get/Set
 *--------------------------------------------------------------------------*/

NwSdpRcT
nwSdpSetUlpEntity( NW_IN NwSdpHandleT hSdp,
                          NW_IN NwSdpUlpEntityT* pUlpEntity)
{
  NwSdpRcT rc;
  NwSdpT* thiz = (NwSdpT*) hSdp;

  if(pUlpEntity)
  {
    thiz->ulp = *(pUlpEntity);
    rc = NW_SDP_OK;
  }
  else
  {
    rc = NW_SDP_FAILURE;
  }

  return rc;
}


NwSdpRcT
nwSdpSetUdpEntity( NW_IN NwSdpHandleT hSdp,
                          NW_IN NwSdpUdpEntityT* pUdpEntity)
{
  NwSdpRcT rc;
  NwSdpT* thiz = (NwSdpT*) hSdp;

  if(pUdpEntity)
  {
    thiz->udp = *(pUdpEntity);
    rc = NW_SDP_OK;
  }
  else
  {
    rc = NW_SDP_FAILURE;
  }

  return rc;
}

NwSdpRcT
nwSdpSetMemMgrEntity( NW_IN NwSdpHandleT hSdp,
                      NW_IN NwSdpMemMgrEntityT* pMemMgrEntity)
{
  NwSdpRcT rc = NW_SDP_OK;
  NwSdpT* thiz = (NwSdpT*) hSdp;

  if(pMemMgrEntity)
  {
    thiz->memMgr = *(pMemMgrEntity);
  }
  else
  {
    rc = NW_SDP_FAILURE;
  }

  return rc;
}

NwSdpRcT
nwSdpSetTimerMgrEntity( NW_IN NwSdpHandleT hSdp,
                        NW_IN NwSdpTimerMgrEntityT* pTmrMgrEntity)
{
  NwSdpRcT rc = NW_SDP_OK;
  NwSdpT* thiz = (NwSdpT*) hSdp;

  if(pTmrMgrEntity)
  {
    thiz->tmrMgr = *(pTmrMgrEntity);
  }
  else
  {
    rc = NW_SDP_FAILURE;
  }

  return rc;
}


NwSdpRcT
nwSdpSetLogMgrEntity( NW_IN NwSdpHandleT hSdp,
                      NW_IN NwSdpLogMgrEntityT* pLogMgrEntity)
{
  NwSdpRcT rc = NW_SDP_OK;
  NwSdpT* thiz = (NwSdpT*) hSdp;

  if(pLogMgrEntity)
  {
    thiz->logMgr = *(pLogMgrEntity);
  }
  else
  {
    rc = NW_SDP_FAILURE;
  }

  return rc;
}

NwSdpRcT
nwSdpSetLogLevel( NW_IN NwSdpHandleT hSdp,
                  NW_IN NwU32T logLevel)
{
  NwSdpT* thiz = (NwSdpT*) hSdp;
  thiz->logLevel = logLevel;

  return NW_SDP_OK;
}

NwSdpRcT
nwSdpSetServiceLogLevel( NW_IN NwSdpHandleT hSdp,
                  NW_IN NwSdpServiceHandleT hService,
                  NW_IN NwU32T logLevel)
{
  NwSdpT* thiz = (NwSdpT*) hSdp;

  return NW_SDP_OK;
}

/*---------------------------------------------------------------------------
 * Process Request from IPv4 Layer
 *--------------------------------------------------------------------------*/

NwSdpRcT 
nwSdpProcessIpv4DataInd( NW_IN NwSdpHandleT hSdp, 
                    NW_IN NwSdpServiceHandleT hIpv4,
                    NW_IN NwU8T* ipv4Buf,
                    NW_IN NwU32T ipv4BufLen)
{
  NwSdpRcT              rc = NW_SDP_OK;
  NwSdpT*          thiz;
  thiz = (NwSdpT*) hSdp;
  NW_ENTER(thiz);
  rc = nwIpv4ProcessLlpDataInd(thiz->hIpv4Stack, ipv4Buf, ipv4BufLen);
  NW_LEAVE(thiz);
  return rc;
}

NwSdpRcT 
nwSdpProcessGreDataInd( NW_IN NwSdpHandleT hSdp, 
                    NW_IN NwSdpServiceHandleT hGre,
                    NW_IN NwCharT* greBuf,
                    NW_IN NwU32T greBufLen,
                    NW_IN NwU16T peerPort,
                    NW_IN NwU32T peerIp)
{
  NwSdpRcT              rc;
  NwSdpT*          thiz;
  thiz = (NwSdpT*) hSdp;
  NW_ENTER(thiz);
  rc = nwGreProcessUdpReq(hGre, greBuf, greBufLen, peerPort, peerIp);
  NW_LEAVE(thiz);
  return rc;
}

NwSdpRcT 
nwSdpProcessGtpuDataInd( NW_IN NwSdpHandleT hSdp, 
                    NW_IN NwU8T* udpData,
                    NW_IN NwU32T udpDataLen,
                    NW_IN NwU16T peerPort,
                    NW_IN NwU32T peerIp)
{
  NwSdpRcT                 rc;
  NwSdpT*          thiz;
  NwU16T                msgType;

  thiz = (NwSdpT*) hSdp;

  NW_ASSERT(thiz);
  NW_ENTER(thiz);
  rc = nwGtpv1uProcessUdpReq(thiz->hGtpv1uStack, udpData, udpDataLen, peerPort, peerIp);
  NW_LEAVE(thiz);
  return rc;
}

NwSdpRcT 
nwSdpProcessUdpDataInd( NW_IN NwSdpHandleT hSdp, 
                    NW_IN NwU8T* udpData,
                    NW_IN NwU32T udpDataLen,
                    NW_IN NwU16T peerPort,
                    NW_IN NwU32T peerIp)
{
  NwSdpRcT              rc;
  NwSdpT*               thiz;

  thiz = (NwSdpT*) hSdp;

  NW_ASSERT(thiz);
  NW_ENTER(thiz);
  /* TODO : Process UDP message */
  NW_LEAVE(thiz);
  return rc;
}


/*---------------------------------------------------------------------------
 * Process Request from Upper Layer
 *--------------------------------------------------------------------------*/

NwSdpRcT
nwSdpProcessUlpReq( NW_IN NwSdpHandleT hSdp,
                    NW_IN NwSdpUlpApiT *pUlpReq)
{
  NwSdpRcT rc;
  NwSdpT* thiz = (NwSdpT*) hSdp;

  NW_ASSERT(thiz);
  NW_ASSERT(pUlpReq != NULL);

  NW_ENTER(thiz);

  switch(pUlpReq->apiType)
  {
    case NW_SDP_ULP_API_CREATE_FLOW:
      {
        NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Received create flow request from ulp");
        rc = nwSdpCreateFlow(thiz, pUlpReq);
      }
      break;

    case NW_SDP_ULP_API_DESTROY_FLOW:
      {
        NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Received destroy flow request from ulp");
        rc = nwSdpDestroyFlow(thiz, pUlpReq);
      }
      break;

    case NW_SDP_ULP_API_UPDATE_FLOW:
      {
        NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Received update flow request from ulp");
        rc = nwSdpUpdateFlow(thiz, pUlpReq);
      }
      break;

    default:
      NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "Unsupported API received from ulp");
      rc = NW_SDP_FAILURE;
      break;
  }

  NW_LEAVE(thiz);

  return rc;
}

/*---------------------------------------------------------------------------
 * Process Timer timeout Request from Timer Manager
 *--------------------------------------------------------------------------*/

NwSdpRcT
nwSdpProcessTimeout(void* timeoutInfo)
{
  NwSdpRcT rc;
  NwSdpT* thiz;

  NW_ASSERT(timeoutInfo != NULL);

  thiz = (NwSdpT*) (((NwSdpTimeoutInfoT*) timeoutInfo)->hStack);

  NW_ASSERT(thiz != NULL);

  NW_ENTER(thiz);
  NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Received timeout event from ULP with timeoutInfo 0x%x!", (NwU32T)timeoutInfo);

  rc = (((NwSdpTimeoutInfoT*) timeoutInfo)->timeoutCallbackFunc) (timeoutInfo);

  NW_LEAVE(thiz);

  return rc;
}

#ifdef __cplusplus
}
#endif

/*--------------------------------------------------------------------------*
 *                      E N D     O F    F I L E                            *
 *--------------------------------------------------------------------------*/


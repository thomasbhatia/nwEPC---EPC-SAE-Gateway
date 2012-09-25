/*----------------------------------------------------------------------------*
 *                                                                            *
 *                               n w - i p v 4                                * 
 *           I n t e r n e t    P r o t o c o l    v 4    S t a c k           *
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
#include <arpa/inet.h>
#include <linux/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>


#include "NwTypes.h"
#include "NwUtils.h"
#include "NwIpv4Error.h"
#include "NwIpv4Private.h"
#include "NwIpv4TunnelEndPoint.h"
#include "NwIpv4.h"
#include "NwIpv4Log.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct NwArpHdr
{
  NwU16T hwType;
  NwU16T protoType;
  NwU8T  hwAddrLen;
  NwU8T  protoAddrLen;
  NwU16T opCode;
  NwU8T  senderMac[6];
  NwU8T  senderIpAddr[4];
  NwU8T  targetMac[6];
  NwU8T  targetIpAddr[4];
} NwArpHdrT;

/*--------------------------------------------------------------------------*
 *                    P R I V A T E    F U N C T I O N S                    *
 *--------------------------------------------------------------------------*/

static void 
nwIpv4DisplayBanner( NwIpv4StackT* thiz)
{
  printf(" *----------------------------------------------------------------------------*\n");
  printf(" *                                                                            *\n");
  printf(" *                               n w - i p v 4                                *\n");
  printf(" *           I n t e r n e t    P r o t o c o l    v 4    S t a c k           *\n");
  printf(" *                                                                            *\n");
  printf(" *                                                                            *\n");
  printf(" * Copyright (c) 2010-2011 Amit Chawre                                        *\n");
  printf(" * All rights reserved.                                                       *\n");
  printf(" *                                                                            *\n");
  printf(" * Redistribution and use in source and binary forms, with or without         *\n");
  printf(" * modification, are permitted provided that the following conditions         *\n");
  printf(" * are met:                                                                   *\n");
  printf(" *                                                                            *\n");
  printf(" * 1. Redistributions of source code must retain the above copyright          *\n");
  printf(" *    notice, this list of conditions and the following disclaimer.           *\n");
  printf(" * 2. Redistributions in binary form must reproduce the above copyright       *\n");
  printf(" *    notice, this list of conditions and the following disclaimer in the     *\n");
  printf(" *    documentation and/or other materials provided with the distribution.    *\n");
  printf(" * 3. The name of the author may not be used to endorse or promote products   *\n");
  printf(" *    derived from this software without specific prior written permission.   *\n");
  printf(" *                                                                            *\n");
  printf(" * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR       *\n");
  printf(" * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES  *\n");
  printf(" * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.    *\n");
  printf(" * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,           *\n");
  printf(" * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT   *\n");
  printf(" * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  *\n");
  printf(" * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY      *\n");
  printf(" * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT        *\n");
  printf(" * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF   *\n");
  printf(" * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.          *\n");
  printf(" *----------------------------------------------------------------------------*\n\n");

}
/*---------------------------------------------------------------------------
 * RBTree Search Functions 
 *--------------------------------------------------------------------------*/

static inline NwS32T
nwIpv4CompareTeid(struct NwIpv4TunnelEndPoint* a, struct NwIpv4TunnelEndPoint* b);

RB_GENERATE(NwIpv4TunnelEndPointIdentifierMap, NwIpv4TunnelEndPoint, sessionMapRbtNode, nwIpv4CompareTeid)

/**
  Comparator funtion for comparing two sessions.

  @param[in] a: Pointer to session a.
  @param[in] b: Pointer to session b.
  @return  An integer greater than, equal to or less than zero according to whether the 
  object pointed to by a is greater than, equal to or less than the object pointed to by b.
 */

static inline NwS32T
nwIpv4CompareTeid(struct NwIpv4TunnelEndPoint* a, struct NwIpv4TunnelEndPoint* b)
{
  //printf("a:"NW_IPV4_ADDR" b:"NW_IPV4_ADDR"\n", NW_IPV4_ADDR_FORMAT(a->ipv4Addr), NW_IPV4_ADDR_FORMAT(b->ipv4Addr));
  if(a->ipv4Addr > b->ipv4Addr)
    return 1;
  if(a->ipv4Addr < b->ipv4Addr)
    return -1;
  return 0;
}

/**
 * Send IPv4 Message Indication to ULP entity.
 *
 * @param[in] hIpv4StackHandle : Stack handle
 * @return NW_IPv4_OK on success.
 */

static NwIpv4RcT
nwIpv4SendUlpMessageIndication( NW_IN NwIpv4StackT* thiz,
    NW_IN NwU32T  apiType,
    NW_IN NwU8T   *pMsgBuf,
    NW_IN NwU16T  msgLength)
{
  NwIpv4RcT rc;
  NwIpv4UlpApiT ulpApi;

  NW_ENTER(thiz);

  ulpApi.apiType                        = apiType;

  if(pMsgBuf && msgLength)
  {
    rc = nwIpv4MsgFromBufferNew((NwIpv4StackHandleT)thiz, pMsgBuf, msgLength, &(ulpApi.apiInfo.recvMsgInfo.hMsg));
    NW_ASSERT(rc == NW_IPv4_OK);
  }

  rc = thiz->ulp.ulpReqCallback(thiz->ulp.hUlp, &ulpApi);
  NW_ASSERT(rc == NW_IPv4_OK);

  NW_LEAVE(thiz);

  return rc;
}


/*---------------------------------------------------------------------------
 * ULP API Processing Functions 
 *--------------------------------------------------------------------------*/

/**
  Process NW_IPv4_ULP_API_CREATE_TUNNEL_ENDPOINT Request from ULP entity.

  @param[in] hIpv4StackHandle : Stack handle
  @param[in] pUlpReq : Pointer to Ulp Req.
  @return NW_IPv4_OK on success.
 */

static NwIpv4RcT
NwIpv4CreateTunnelEndPoint( NW_IN  NwIpv4StackT* thiz,  
                    NW_IN  NwU32T ipv4Addr, 
                    NW_IN  NwIpv4UlpSessionHandleT hUlpSession, 
                    NW_OUT NwIpv4StackSessionHandleT *phStackSession )
{
  NwIpv4RcT rc = NW_IPv4_OK;
  NwIpv4TunnelEndPointT* pTunnelEndPoint;
  NwIpv4TunnelEndPointT* pCollision;

  NW_ENTER(thiz);

  pTunnelEndPoint = nwIpv4TunnelEndPointNew(thiz);

  if(pTunnelEndPoint)
  {

    pTunnelEndPoint->ipv4Addr       = ipv4Addr;
    pTunnelEndPoint->pStack         = thiz;
    pTunnelEndPoint->hUlpSession    = hUlpSession;

    pCollision = RB_INSERT(NwIpv4TunnelEndPointIdentifierMap, &(thiz->ipv4AddrMap), pTunnelEndPoint);

    if(pCollision)
    {
      NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "Tunnel end-point cannot be created for ipv4Addr 0x%x. TEID already exists", ipv4Addr);
      rc = nwIpv4TunnelEndPointDestroy(thiz, pTunnelEndPoint);
      NW_ASSERT(rc == NW_IPv4_OK);
      *phStackSession = (NwIpv4StackSessionHandleT) 0; 
      rc = NW_IPv4_FAILURE;
    }
    else
    {
      NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Tunnel end-point '0x%x' creation successful for ipv4Addr "NW_IPV4_ADDR, pTunnelEndPoint, NW_IPV4_ADDR_FORMAT(ipv4Addr));

      /* Send GARP Request */
      NwU8T bcastMac[6] = { 0xFF, 0xFF, 0xFF , 0xFF, 0xFF, 0xFF };
      if(thiz->llp.llpArpDataReqCallback)
      {
        rc = thiz->llp.llpArpDataReqCallback(thiz->llp.hLlp,
            ARPOP_REQUEST, 
            bcastMac,
            (NwU8T*)&ipv4Addr,
            (NwU8T*)&ipv4Addr);
      }

      *phStackSession = (NwIpv4StackSessionHandleT) pTunnelEndPoint; 
    }
  }
  else
  {
    *phStackSession = (NwIpv4StackSessionHandleT) 0; 
    rc = NW_IPv4_FAILURE;
  }

  NW_LEAVE(thiz);
  return rc;
}

/**
  Process NW_IPv4_ULP_API_DESTROY_TUNNEL_ENDPOINT Request from ULP entity.

  @param[in] hIpv4StackHandle : Stack handle
  @param[in] pUlpReq : Pointer to Ulp Req.
  @return NW_IPv4_OK on success.
 */

static NwIpv4RcT
nwIpv4DestroyTunnelEndPoint( NwIpv4StackT* thiz,  NW_IN NwIpv4UlpApiT *pUlpReq)
{
  NwIpv4RcT rc = NW_IPv4_OK;
  NwIpv4TunnelEndPointT *pRemovedTeid;
  NwIpv4StackSessionHandleT hStackSessionHandle;

  hStackSessionHandle = pUlpReq->apiInfo.destroyTunnelEndPointInfo.hStackSessionHandle;
  NW_ASSERT(hStackSessionHandle);

  NW_LOG(thiz, NW_LOG_LEVEL_INFO, "Destroying Tunnel end-point '%x'", hStackSessionHandle);
  pRemovedTeid = RB_REMOVE(NwIpv4TunnelEndPointIdentifierMap, &(thiz->ipv4AddrMap), (NwIpv4TunnelEndPointT*)(hStackSessionHandle));

  NW_ASSERT(pRemovedTeid == (NwIpv4TunnelEndPointT*)(hStackSessionHandle));

  rc = nwIpv4TunnelEndPointDestroy(thiz, (NwIpv4TunnelEndPointT*) hStackSessionHandle);

  return rc;
}

/**
  Process NW_IPv4_ULP_API_SEND_TPDU Request from ULP entity.

  @param[in] thiz: Stack handle
  @param[in] pUlpReq : Pointer to Ulp Req.
  @return NW_IPv4_OK on success.
 */

static NwIpv4RcT
nwIpv4Sendto( NwIpv4StackT* thiz,  NW_IN NwIpv4UlpApiT *pUlpReq)
{
  NwIpv4RcT rc;
  NwU8T* ipv4Hdr;
  NwIpv4MsgT*  pMsg;

  pMsg = (NwIpv4MsgT*) pUlpReq->apiInfo.sendtoInfo.hMsg;

  NW_ASSERT(thiz);
  NW_ASSERT(pMsg);

  NW_ENTER(thiz);

  /* TODO: Send the mesasge over wire */
  rc = thiz->llp.llpDataReqCallback(thiz->llp.hLlp,
      pMsg->msgBuf,
      pMsg->msgLen);

  NW_LEAVE(thiz);
  return rc;
}

/**
  Process PDU from LLP entity.

  @param[in] thiz: Stack handle
  @return NW_IPv4_OK on success.
 */

static NwIpv4RcT
nwIpv4ProcessPdu( NwIpv4StackT* thiz, 
                  NW_IN NwCharT* pdu,
                  NW_IN NwU32T gPduLen)

{
  NwIpv4RcT rc;
  NwIpv4TunnelEndPointT* pTunnelEndPoint;
  NwIpv4TunnelEndPointT tunnelEndPointKey;

  NW_ENTER(thiz);

  if(*(NwU16T*)(pdu + 12) == htons(ETH_P_IP))
  {

    typedef struct NwIpv4Hdr
    {
      NwU8T  ch[8];
      NwU8T  ttl;
      NwU8T  protocol;
      NwU16T csum;
      NwU32T srcAddr;
      NwU32T dstAddr;
    } NwIpv4HdrT;

    NwIpv4HdrT *pHdr = (NwIpv4HdrT*) (pdu + 14);
    gPduLen -= 14;

    NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Received IP-PDU : ");
    NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Version              - %u", (pHdr->ch[0] & 0xf0) >> 4);
    NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Header Length        - %u", pHdr->ch[0] & 0x0f);
    NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Protocol             - %u", pHdr->protocol);
    NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Source IP            - %d.%d.%d.%d", 
        (pHdr->srcAddr & 0x000000FF),
        (pHdr->srcAddr & 0x0000FF00) >> 8, 
        (pHdr->srcAddr & 0x00FF0000) >> 16, 
        (pHdr->srcAddr & 0xFF000000) >> 24); 
    NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Destination IP       - %d.%d.%d.%d", 
        (pHdr->dstAddr & 0x000000FF),
        (pHdr->dstAddr & 0x0000FF00) >> 8, 
        (pHdr->dstAddr & 0x00FF0000) >> 16, 
        (pHdr->dstAddr & 0xFF000000) >> 24); 


    tunnelEndPointKey.ipv4Addr = (thiz->mode == NW_IPv4_MODE_DOWNLINK ? pHdr->dstAddr : pHdr->srcAddr);
    pTunnelEndPoint = RB_FIND(NwIpv4TunnelEndPointIdentifierMap, &(thiz->ipv4AddrMap), &tunnelEndPointKey);

    if(pTunnelEndPoint)
    {
      NwIpv4MsgHandleT hMsg;

      NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Received IP PDU for end-point " NW_IPV4_ADDR " from " NW_IPV4_ADDR, NW_IPV4_ADDR_FORMAT(tunnelEndPointKey.ipv4Addr), NW_IPV4_ADDR_FORMAT(pHdr->srcAddr));
      rc = nwIpv4SessionSendMsgApiToUlpEntity(pTunnelEndPoint, pHdr, gPduLen);
    }
    else
    {
      NW_LOG(thiz, NW_LOG_LEVEL_INFO, "Received IP PDU over non-existent tunnel end-point " NW_IPV4_ADDR " from " NW_IPV4_ADDR, NW_IPV4_ADDR_FORMAT(tunnelEndPointKey.ipv4Addr), NW_IPV4_ADDR_FORMAT(pHdr->srcAddr));
    }
  }
  else if(*(NwU16T*)(pdu + 12) == htons(ETH_P_ARP))
  {

    NwArpHdrT *pHdr = (NwArpHdrT*) (pdu + 14);
    gPduLen -= 14;

    NW_LOG(thiz, NW_LOG_LEVEL_DEBG,"Received ARP request:");
    NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "HW Address Type            - %u", ntohs(pHdr->hwType));
    NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Protocol Address Type      - %u", ntohs(pHdr->protoType));
    NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "HW Address Length          - %u", pHdr->hwAddrLen);
    NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Protocol Address Length    - %u", pHdr->protoAddrLen);
    NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Op Code                    - %u", ntohs(pHdr->opCode));
    NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Sender IP Address          - %d.%d.%d.%d", 
        (pHdr->senderIpAddr[0]),
        (pHdr->senderIpAddr[1]) , 
        (pHdr->senderIpAddr[2]) , 
        (pHdr->senderIpAddr[3])); 
    NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Target IP                  - %d.%d.%d.%d", 
        (pHdr->targetIpAddr[0]),
        (pHdr->targetIpAddr[1]), 
        (pHdr->targetIpAddr[2]), 
        (pHdr->targetIpAddr[3])); 

    tunnelEndPointKey.ipv4Addr = (NwU32T)(pHdr->targetIpAddr);
    pTunnelEndPoint = RB_FIND(NwIpv4TunnelEndPointIdentifierMap, &(thiz->ipv4AddrMap), &tunnelEndPointKey);
    if(pTunnelEndPoint)
    {
      rc = thiz->llp.llpArpDataReqCallback(thiz->llp.hLlp,
          ARPOP_REPLY, 
          pHdr->senderMac,
          pHdr->targetIpAddr,
          pHdr->senderIpAddr);

      NW_LOG(thiz, NW_LOG_LEVEL_ERRO,"Sending ARP reponse!");
    }
  }
  else
  {
    NW_LOG(thiz, NW_LOG_LEVEL_ERRO,"Unhandle Layer 3 Protocol Type 0x%x!", *(NwU16T*)(pdu + 12));
  }

  NW_LEAVE(thiz);

  return rc;
}


/*--------------------------------------------------------------------------*
 *                     P U B L I C   F U N C T I O N S                      *
 *--------------------------------------------------------------------------*/

NwCharT* ipv4LogLevelStr[] = {"EMER", "ALER", "CRIT",  "ERRO", "WARN", "NOTI", "INFO", "DEBG"};

/*---------------------------------------------------------------------------
 * Constructor
 *--------------------------------------------------------------------------*/

NwIpv4RcT
nwIpv4Initialize( NW_INOUT NwIpv4StackHandleT* hIpv4StackHandle)
{
  NwIpv4RcT rc;
  NwIpv4StackT* thiz;

  thiz = (NwIpv4StackT*) malloc( sizeof(NwIpv4StackT));

  if(thiz)
  {
    thiz->id    = (NwU32T) thiz;
    thiz->seq   = (NwU16T) ((NwU32T)thiz) ;
    thiz->mode  = NW_IPv4_MODE_DOWNLINK;
    RB_INIT(&(thiz->ipv4AddrMap));
#if 0
    nwIpv4DisplayBanner(thiz);
#endif

    rc = NW_IPv4_OK;
  }
  else
  {
    rc = NW_IPv4_FAILURE;
  }


  *hIpv4StackHandle = (NwIpv4StackHandleT) thiz;
  return rc;
}


/*---------------------------------------------------------------------------
 * Destructor
 *--------------------------------------------------------------------------*/

NwIpv4RcT
nwIpv4Finalize( NW_IN  NwIpv4StackHandleT hIpv4StackHandle)
{
  NwIpv4RcT rc;
  if(hIpv4StackHandle)
  {
    free((void*)hIpv4StackHandle);
    rc = NW_IPv4_OK;
  }
  else
  {
    rc = NW_IPv4_FAILURE;
  }
  return rc;
}


/*---------------------------------------------------------------------------
 * Configuration Get/Set
 *--------------------------------------------------------------------------*/

NwIpv4RcT
nwIpv4SetMode( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
               NW_IN NwU32T             mode)
{
  NwIpv4RcT rc;
  NwIpv4StackT* thiz = (NwIpv4StackT*) hIpv4StackHandle;
  thiz->mode = mode;
  return NW_IPv4_OK;
}

NwIpv4RcT
nwIpv4SetUlpEntity( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
                          NW_IN NwIpv4UlpEntityT* pUlpEntity)
{
  NwIpv4RcT rc;
  NwIpv4StackT* thiz = (NwIpv4StackT*) hIpv4StackHandle;

  if(pUlpEntity)
  {
    thiz->ulp = *(pUlpEntity);
    rc = NW_IPv4_OK;
  }
  else
  {
    rc = NW_IPv4_FAILURE;
  }

  return rc;
}


NwIpv4RcT
nwIpv4SetLlpEntity( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
                          NW_IN NwIpv4LlpEntityT* pLlpEntity)
{
  NwIpv4RcT rc;
  NwIpv4StackT* thiz = (NwIpv4StackT*) hIpv4StackHandle;

  if(pLlpEntity)
  {
    thiz->llp = *(pLlpEntity);
    rc = NW_IPv4_OK;
  }
  else
  {
    rc = NW_IPv4_FAILURE;
  }

  return rc;
}

NwIpv4RcT
nwIpv4SetMemMgrEntity( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
                               NW_IN NwIpv4MemMgrEntityT* pMemMgrEntity)
{
  NwIpv4RcT rc;
  NwIpv4StackT* thiz = (NwIpv4StackT*) hIpv4StackHandle;

  if(pMemMgrEntity)
  {
    thiz->memMgr = *(pMemMgrEntity);
    rc = NW_IPv4_OK;
  }
  else
  {
    rc = NW_IPv4_FAILURE;
  }

  return rc;
}


NwIpv4RcT
nwIpv4SetTimerMgrEntity( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
                               NW_IN NwIpv4TimerMgrEntityT* pTmrMgrEntity)
{
  NwIpv4RcT rc;
  NwIpv4StackT* thiz = (NwIpv4StackT*) hIpv4StackHandle;

  if(pTmrMgrEntity)
  {
    thiz->tmrMgr = *(pTmrMgrEntity);
    rc = NW_IPv4_OK;
  }
  else
  {
    rc = NW_IPv4_FAILURE;
  }

  return rc;
}


NwIpv4RcT
nwIpv4SetLogMgrEntity( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
                             NW_IN NwIpv4LogMgrEntityT* pLogMgrEntity)
{
  NwIpv4RcT rc;
  NwIpv4StackT* thiz = (NwIpv4StackT*) hIpv4StackHandle;
 
  if(pLogMgrEntity)
  {
    thiz->logMgr = *(pLogMgrEntity);
    rc = NW_IPv4_OK;
  }
  else
  {
    rc = NW_IPv4_FAILURE;
  }
 return rc;
}

NwIpv4RcT
nwIpv4SetLogLevel( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
                         NW_IN NwU32T logLevel)
{
  NwIpv4StackT* thiz = (NwIpv4StackT*) hIpv4StackHandle;
  thiz->logLevel = logLevel;
}

/*---------------------------------------------------------------------------
 * Process Request from LLP Layer
 *--------------------------------------------------------------------------*/

NwIpv4RcT 
nwIpv4ProcessLlpDataInd( NW_IN NwIpv4StackHandleT hIpv4StackHandle, 
                    NW_IN NwU8T* data,
                    NW_IN NwU32T dataLen)
{
  return nwIpv4ProcessPdu((NwIpv4StackT*) hIpv4StackHandle, data, dataLen);
}

/*---------------------------------------------------------------------------
 * Process Request from Upper Layer
 *--------------------------------------------------------------------------*/

NwIpv4RcT
nwIpv4ProcessUlpReq( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
                    NW_IN NwIpv4UlpApiT *pUlpReq)
{
  NwIpv4RcT rc;
  NwIpv4StackT* thiz = (NwIpv4StackT*) hIpv4StackHandle;

  NW_ASSERT(thiz);
  NW_ASSERT(pUlpReq != NULL);

  NW_ENTER(thiz);

  switch(pUlpReq->apiType)
  {
    case NW_IPv4_ULP_API_CREATE_TUNNEL_ENDPOINT:
      {
        NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Received create session req from ulp");
        rc = NwIpv4CreateTunnelEndPoint(thiz, 
            pUlpReq->apiInfo.createTunnelEndPointInfo.ipv4Addr,
            pUlpReq->apiInfo.createTunnelEndPointInfo.hUlpSession,
            &(pUlpReq->apiInfo.createTunnelEndPointInfo.hStackSession));
      }
      break;

    case NW_IPv4_ULP_API_DESTROY_TUNNEL_ENDPOINT:
      {
        NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Received destroy session req from ulp");
        rc = nwIpv4DestroyTunnelEndPoint(thiz,  pUlpReq);
      }
      break;

    case NW_IPv4_ULP_API_SEND_TPDU:
      {
        NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Received send tpdu req from ulp");
        rc = nwIpv4Sendto(thiz,  pUlpReq);
      }
      break;

    default:
      NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "Unsupported API received from ulp");
      rc = NW_IPv4_FAILURE;
      break;
  }

  NW_LEAVE(thiz);

  return rc;
}

/*---------------------------------------------------------------------------
 * Process Timer timeout Request from Timer Manager
 *--------------------------------------------------------------------------*/

NwIpv4RcT
nwIpv4ProcessTimeout(void* timeoutInfo)
{
  NwIpv4RcT rc;
  NwIpv4StackT* thiz;

  NW_ASSERT(timeoutInfo != NULL);

  thiz = (NwIpv4StackT*) (((NwIpv4TimeoutInfoT*) timeoutInfo)->hStack);

  NW_ASSERT(thiz != NULL);

  NW_ENTER(thiz);
  NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Received timeout event from ULP with timeoutInfo %x!", timeoutInfo);

  rc = (((NwIpv4TimeoutInfoT*) timeoutInfo)->timeoutCallbackFunc) (timeoutInfo);

  NW_LEAVE(thiz);

  return rc;
}

#ifdef __cplusplus
}
#endif

/*--------------------------------------------------------------------------*
 *                      E N D     O F    F I L E                            *
 *--------------------------------------------------------------------------*/


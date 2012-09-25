/*----------------------------------------------------------------------------*
 *                                                                            *
 *                             n w - g t p v 2 u                              * 
 *  G e n e r i c    R o u t i n g    E n c a p s u l a t i o n    S t a c k  *
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

#include "NwTypes.h"
#include "NwUtils.h"
#include "NwGreError.h"
#include "NwGrePrivate.h"
#include "NwGreTunnelEndPoint.h"
#include "NwGre.h"
#include "NwGreLog.h"

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------*
 *                    P R I V A T E    F U N C T I O N S                    *
 *--------------------------------------------------------------------------*/

static void 
nwGreDisplayBanner( NwGreStackT* thiz)
{
  printf(" *----------------------------------------------------------------------------*\n");
  printf(" *                                                                            *\n");
  printf(" *                                n w - g r e                                 *\n");
  printf(" *  G e n e r i c    R o u t i n g    E n c a p s u l a t i o n    S t a c k  *\n");
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
nwGreCompareGreKey(struct NwGreTunnelEndPoint* a, struct NwGreTunnelEndPoint* b);

static inline NwS32T
nwGreCompareSeqNum(struct NwGreTrxn* a, struct NwGreTrxn* b);

RB_GENERATE(NwGreOutstandingTxSeqNumTrxnMap, NwGreTrxn, outstandingTxSeqNumMapRbtNode, nwGreCompareSeqNum)
RB_GENERATE(NwGreOutstandingRxSeqNumTrxnMap, NwGreTrxn, outstandingRxSeqNumMapRbtNode, nwGreCompareSeqNum)
RB_GENERATE(NwGreTunnelEndPointTMap, NwGreTunnelEndPoint, sessionMapRbtNode, nwGreCompareGreKey)
RB_GENERATE(NwGreTunnelEndPointIdentifierMap, NwGreTunnelEndPoint, sessionMapRbtNode, nwGreCompareGreKey)

/**
  Comparator funtion for comparing two sessions.

  @param[in] a: Pointer to session a.
  @param[in] b: Pointer to session b.
  @return  An integer greater than, equal to or less than zero according to whether the 
  object pointed to by a is greater than, equal to or less than the object pointed to by b.
 */

static inline NwS32T
nwGreCompareGreKey(struct NwGreTunnelEndPoint* a, struct NwGreTunnelEndPoint* b)
{
  if(a->greKey > b->greKey)
    return 1;
  if(a->greKey < b->greKey)
    return -1;
  return 0;
}

/**
  Comparator funtion for comparing two sequence number transactions.

  @param[in] a: Pointer to session a.
  @param[in] b: Pointer to session b.
  @return  An integer greater than, equal to or less than zero according to whether the 
  object pointed to by a is greater than, equal to or less than the object pointed to by b.
 */

static inline NwS32T
nwGreCompareSeqNum(struct NwGreTrxn* a, struct NwGreTrxn* b)
{
  if(a->seqNum > b->seqNum)
    return 1;
  if(a->seqNum < b->seqNum)
    return -1;
  if(a->peerIp > b->peerIp) 
    return 1;
  if(a->peerIp < b->peerIp) 
    return -1;
  return 0;
}

/**
 * Send GRE Message Indication to ULP entity.
 *
 * @param[in] hGreStackHandle : Stack handle
 * @return NW_OK on success.
 */

static NwRcT
nwGreSendUlpMessageIndication( NW_IN NwGreStackT* thiz,
    NW_IN NwU32T  hUlpTrxn,
    NW_IN NwU32T  apiType,
    NW_IN NwU32T  msgType,
    NW_IN NwU32T  peerIp,
    NW_IN NwU16T  peerPort,
    NW_IN NwU8T   *pMsgBuf,
    NW_IN NwU16T  msgLength)
{
  NwRcT rc;
  NwGreUlpApiT ulpApi;

  NW_ENTER(thiz);

  ulpApi.apiType                        = apiType;
  ulpApi.apiInfo.recvMsgInfo.msgType    = msgType;
  ulpApi.apiInfo.recvMsgInfo.hUlpTrxn   = hUlpTrxn;
  ulpApi.apiInfo.recvMsgInfo.peerIp     = peerIp;
  ulpApi.apiInfo.recvMsgInfo.peerPort   = peerPort;

  if(pMsgBuf && msgLength)
  {
    rc = nwGreMsgFromBufferNew((NwGreStackHandleT)thiz, pMsgBuf, msgLength, &(ulpApi.apiInfo.recvMsgInfo.hMsg));
    NW_ASSERT(NW_OK == rc);
  }

  rc = thiz->ulp.ulpReqCallback(thiz->ulp.hUlp, &ulpApi);
  NW_ASSERT(NW_OK == rc);

  NW_LEAVE(thiz);

  return rc;
}

NwRcT
nwGrePeerRspTimeout(void* arg)
{
  NwRcT rc = NW_OK;
  NwGreTrxnT* thiz;
  NwGreTimeoutInfoT *timeoutInfo = arg;

  printf("Retransmission timer expired\n");

  thiz = ((NwGreTrxnT*)timeoutInfo->timeoutArg);
  rc = thiz->pStack->udp.udpDataReqCallback(thiz->pStack->udp.hUdp,
      thiz->pMsg->msgBuf,
      thiz->pMsg->msgLen,
      thiz->peerIp,
      thiz->peerPort);

  if(thiz->maxRetries)
  {
    rc = thiz->pStack->tmrMgr.tmrStartCallback(thiz->pStack->tmrMgr.tmrMgrHandle, 5, 0, NW_GRE_TMR_TYPE_ONE_SHOT, (void*)timeoutInfo, &thiz->hRspTmr);
    thiz->maxRetries--;
  }
  else
  {
    /* Inform session layer about path fialure */
    printf("Max retries over!\n");
  }
  return rc;
}

/*---------------------------------------------------------------------------
 * ULP API Processing Functions 
 *--------------------------------------------------------------------------*/

/**
  Process NW_GRE_ULP_API_CREATE_TUNNEL_ENDPOINT Request from ULP entity.

  @param[in] hGreStackHandle : Stack handle
  @param[in] pUlpReq : Pointer to Ulp Req.
  @return NW_OK on success.
 */

static NwRcT
NwGreCreateTunnelEndPoint( NW_IN  NwGreStackT* thiz,  
                    NW_IN  NwU32T greKey, 
                    NW_IN  NwGreUlpSessionHandleT hUlpSession, 
                    NW_OUT NwGreStackSessionHandleT *phStackSession )
{
  NwRcT rc = NW_OK;
  NwGreTunnelEndPointT* pTunnelEndPoint;
  NwGreTunnelEndPointT* pCollision;

  NW_ENTER(thiz);

  pTunnelEndPoint = nwGreTunnelEndPointNew(thiz);

  if(pTunnelEndPoint)
  {

    NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Tunnel end-point '0x%x' creation successful for GRE Key 0x%x", pTunnelEndPoint, greKey);

    pTunnelEndPoint->greKey             = greKey;
    pTunnelEndPoint->pStack             = thiz;
    pTunnelEndPoint->hUlpSession        = hUlpSession;

    pCollision = RB_INSERT(NwGreTunnelEndPointIdentifierMap, &(thiz->teidMap), pTunnelEndPoint);

    if(pCollision)
    {
      NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "Tunnel end-point cannot be created for GRE Key 0x%x. GRE Key already exists", pTunnelEndPoint, greKey);
      rc = nwGreTunnelEndPointDestroy(thiz, pTunnelEndPoint);
      NW_ASSERT(NW_OK == rc);
      rc = NW_FAILURE;
    }
    else
    {
      *phStackSession = (NwGreStackSessionHandleT) pTunnelEndPoint; 
    }

  }
  else
  {
    rc = NW_FAILURE;
  }

  NW_LEAVE(thiz);
  return rc;
}

/**
  Process NW_GRE_ULP_API_DESTROY_TUNNEL_ENDPOINT Request from ULP entity.

  @param[in] hGreStackHandle : Stack handle
  @param[in] pUlpReq : Pointer to Ulp Req.
  @return NW_OK on success.
 */

static NwRcT
nwGreDestroyTunnelEndPoint( NwGreStackT* thiz,  NW_IN NwGreUlpApiT *pUlpReq)
{
  NwRcT rc = NW_OK;
  NwGreTunnelEndPointT *pRemovedTunnel;

  NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Destroying Tunnel end-point '%x'", pUlpReq->apiInfo.destroyTunnelEndPointInfo.hStackSessionHandle);
  pRemovedTunnel = RB_REMOVE(NwGreTunnelEndPointIdentifierMap, &(thiz->teidMap), (NwGreTunnelEndPointT*)(pUlpReq->apiInfo.destroyTunnelEndPointInfo.hStackSessionHandle));

  NW_ASSERT(pRemovedTunnel == (NwGreTunnelEndPointT*)(pUlpReq->apiInfo.destroyTunnelEndPointInfo.hStackSessionHandle));

  rc = nwGreTunnelEndPointDestroy(thiz, (NwGreTunnelEndPointT*) pUlpReq->apiInfo.destroyTunnelEndPointInfo.hStackSessionHandle);

  return rc;
}

/**
  Process NW_GRE_ULP_API_INITIAL_REQ Request from ULP entity.

  @param[in] hGreStackHandle : Stack handle
  @param[in] pUlpReq : Pointer to Ulp Req.
  @return NW_OK on success.
 */

static NwRcT
nwGreInitialReq( NW_IN NwGreStackT* thiz, NW_IN NwGreUlpApiT *pUlpReq)
{
  NwRcT rc;
  NwGreTrxnT *pTrxn;

  NW_ENTER(thiz);

  /* Create New Transaction */
  rc = nwGreTrxnNew(thiz, &pTrxn);

  if(pTrxn)
  {
    rc = nwGreTrxnCreateAndSendMsg(thiz,
        pTrxn,
        pUlpReq->apiInfo.initialReqInfo.peerIp,
        pUlpReq->apiInfo.initialReqInfo.peerPort,
        (NwGreMsgT*) pUlpReq->apiInfo.initialReqInfo.hMsg);

    if(NW_OK == rc)
    {
      /* Insert into search tree */
      RB_INSERT(NwGreOutstandingTxSeqNumTrxnMap, &(thiz->outstandingTxSeqNumMap), pTrxn);
    }
    else
    {
      rc = nwGreTrxnDelete(&pTrxn);
      NW_ASSERT(NW_OK == rc);
    }
  }

  NW_LEAVE(thiz);

  return rc;
}

/**
  Process NW_GRE_ULP_API_SEND_TPDU Request from ULP entity.

  @param[in] thiz: Stack handle
  @param[in] pUlpReq : Pointer to Ulp Req.
  @return NW_OK on success.
 */

static NwRcT
nwGreSendto( NwGreStackT* thiz,  NW_IN NwGreUlpApiT *pUlpReq)
{
  NwRcT rc;
  NwU8T* greHdr;
  NwGreMsgT*  pMsg;

  pMsg = (NwGreMsgT*) pUlpReq->apiInfo.sendtoInfo.hMsg;

  NW_ASSERT(thiz);
  NW_ASSERT(pMsg);

  NW_ENTER(thiz);

  greHdr = pMsg->msgBuf;
  NW_ASSERT(greHdr != NULL);

  *(greHdr++)         = (pMsg->csumPresent << 7)         | 
                        (pMsg->keyPresent << 5)          | 
                        (pMsg->seqNumPresent << 4);

  *(greHdr++)         = (pMsg->version & 0x07);
  *((NwU16T*) greHdr) = htons(pMsg->protocolType);
  greHdr += 2;

  if(pMsg->csumPresent)
  {
    *((NwU16T*) greHdr) = htonl(0x0000);
    greHdr += 2;
    *((NwU16T*) greHdr) = (0x0000);
    greHdr += 2;
  }

  if(pMsg->keyPresent)
  {
    *((NwU32T*) greHdr) = htonl(pMsg->greKey);
    greHdr += 4;
  }

  if(pMsg->seqNumPresent)
  {
    *((NwU32T*) greHdr) = pMsg->seqNumPresent;
  }

  NW_ASSERT(thiz->udp.udpDataReqCallback != NULL);

  rc = thiz->udp.udpDataReqCallback(thiz->udp.hUdp,
      pMsg->msgBuf,
      pMsg->msgLen,
      pUlpReq->apiInfo.sendtoInfo.ipAddr,
      2152);

  NW_LEAVE(thiz);
  return rc;
}

/**
  Process GPDU from UDP entity.

  @param[in] thiz: Stack handle
  @param[in] pUlpReq : Pointer to Ulp Req.
  @return NW_OK on success.
 */

static NwRcT
nwGreProcessGpdu( NwGreStackT* thiz, 
                     NW_IN NwCharT* gpdu,
                     NW_IN NwU32T gdpuLen,
                     NW_IN NwU32T peerIp)

{
  NwRcT rc;
  NwGreTunnelEndPointT* pTunnelEndPoint;
  NwGreTunnelEndPointT tunnelEndPointKey;

  NW_ENTER(thiz);

  /* Skip IP header */
  gpdu += 20;

  if(*gpdu & 0x20)
  {
    tunnelEndPointKey.greKey = (*gpdu & 0x80 ? ntohl((NwU32T)*((NwU32T*)(gpdu + 8))) : ntohl((NwU32T)*((NwU32T*)(gpdu + 4))));
    pTunnelEndPoint = RB_FIND(NwGreTunnelEndPointIdentifierMap, &(thiz->teidMap), &tunnelEndPointKey);

    if(pTunnelEndPoint)
    {
      NwGreMsgHandleT hMsg;

      rc = nwGreMsgFromBufferNew( (NwGreStackHandleT)thiz,
          gpdu,
          gdpuLen - 20,
          &hMsg);

      if(NW_OK == rc)
      {
        NwGreMsgT* pMsg = (NwGreMsgT*) hMsg;
        NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Received T-PDU over tunnel end-point '%x' of size %u", tunnelEndPointKey.greKey, pMsg->msgLen);
        rc = nwGreSessionSendMsgApiToUlpEntity(pTunnelEndPoint, pMsg);
      }
    }
    else
    {
      NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "Received T-PDU over non-existent tunnel end-point '%x' from 0x%x", tunnelEndPointKey.greKey, ntohl(peerIp));
    }
  }
  else
  {
    NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "Received T-PDU with key flag from 0x%x", *gpdu, ntohl(peerIp));
  }
  NW_LEAVE(thiz);

  return rc;
}


/*--------------------------------------------------------------------------*
 *                     P U B L I C   F U N C T I O N S                      *
 *--------------------------------------------------------------------------*/

NwCharT* greLogLevelStr[] = {"EMER", "ALER", "CRIT",  "ERRO", "WARN", "NOTI", "INFO", "DEBG"};

/*---------------------------------------------------------------------------
 * Constructor
 *--------------------------------------------------------------------------*/

NwRcT
nwGreInitialize( NW_INOUT NwGreStackHandleT* hGreStackHandle)
{
  NwRcT rc;
  NwGreStackT* thiz;

  thiz = (NwGreStackT*) malloc (sizeof(NwGreStackT));

  if(thiz)
  {
    thiz->id    = (NwU32T) thiz;
    thiz->seq   = (NwU16T) ((NwU32T)thiz) ;
    RB_INIT(&(thiz->outstandingTxSeqNumMap));
    RB_INIT(&(thiz->outstandingRxSeqNumMap));
    nwGreDisplayBanner(thiz);

    rc = NW_OK;
  }
  else
  {
    rc = NW_FAILURE;
  }


  *hGreStackHandle = (NwGreStackHandleT) thiz;
  return rc;
}


/*---------------------------------------------------------------------------
 * Destructor
 *--------------------------------------------------------------------------*/

NwRcT
nwGreFinalize( NW_IN  NwGreStackHandleT hGreStackHandle)
{
  NwRcT rc;
  if(hGreStackHandle)
  {
    free((void*)hGreStackHandle);
    rc = NW_OK;
  }
  else
  {
    rc = NW_FAILURE;
  }
  return rc;
}


/*---------------------------------------------------------------------------
 * Configuration Get/Set
 *--------------------------------------------------------------------------*/

NwRcT
nwGreSetUlpEntity( NW_IN NwGreStackHandleT hGreStackHandle,
                          NW_IN NwGreUlpEntityT* pUlpEntity)
{
  NwRcT rc;
  NwGreStackT* thiz = (NwGreStackT*) hGreStackHandle;

  if(pUlpEntity)
  {
    thiz->ulp = *(pUlpEntity);
    rc = NW_OK;
  }
  else
  {
    rc = NW_FAILURE;
  }

  return rc;
}


NwRcT
nwGreSetUdpEntity( NW_IN NwGreStackHandleT hGreStackHandle,
                          NW_IN NwGreLlpEntityT* pUdpEntity)
{
  NwRcT rc;
  NwGreStackT* thiz = (NwGreStackT*) hGreStackHandle;

  if(pUdpEntity)
  {
    thiz->udp = *(pUdpEntity);
    rc = NW_OK;
  }
  else
  {
    rc = NW_FAILURE;
  }

  return rc;
}


NwRcT
nwGreSetTimerMgrEntity( NW_IN NwGreStackHandleT hGreStackHandle,
                               NW_IN NwGreTimerMgrEntityT* pTmrMgrEntity)
{
  NwRcT rc;
  NwGreStackT* thiz = (NwGreStackT*) hGreStackHandle;

  if(pTmrMgrEntity)
  {
    thiz->tmrMgr = *(pTmrMgrEntity);
    rc = NW_OK;
  }
  else
  {
    rc = NW_FAILURE;
  }

  return rc;
}


NwRcT
nwGreSetLogMgrEntity( NW_IN NwGreStackHandleT hGreStackHandle,
                             NW_IN NwGreLogMgrEntityT* pLogMgrEntity)
{
  NwRcT rc;
  NwGreStackT* thiz = (NwGreStackT*) hGreStackHandle;
 
  if(pLogMgrEntity)
  {
    thiz->logMgr = *(pLogMgrEntity);
    rc = NW_OK;
  }
  else
  {
    rc = NW_FAILURE;
  }
 return rc;
}

NwRcT
nwGreSetLogLevel( NW_IN NwGreStackHandleT hGreStackHandle,
                         NW_IN NwU32T logLevel)
{
  NwGreStackT* thiz = (NwGreStackT*) hGreStackHandle;
  thiz->logLevel = logLevel;
}

/*---------------------------------------------------------------------------
 * Process Request from Udp Layer
 *--------------------------------------------------------------------------*/

NwRcT 
nwGreProcessUdpReq( NW_IN NwGreStackHandleT hGreStackHandle, 
                    NW_IN NwCharT* udpData,
                    NW_IN NwU32T udpDataLen,
                    NW_IN NwU16T peerPort,
                    NW_IN NwU32T peerIp)
{
  NwRcT                 rc;
  NwGreStackT*       thiz;
  NwU16T                msgType;

  thiz = (NwGreStackT*) hGreStackHandle;

  NW_ASSERT(thiz);

  rc = nwGreProcessGpdu(thiz, udpData, udpDataLen, peerIp);

  NW_LEAVE(thiz);
  return rc;
}


/*---------------------------------------------------------------------------
 * Process Request from Upper Layer
 *--------------------------------------------------------------------------*/

NwRcT
nwGreProcessUlpReq( NW_IN NwGreStackHandleT hGreStackHandle,
                    NW_IN NwGreUlpApiT *pUlpReq)
{
  NwRcT rc;
  NwGreStackT* thiz = (NwGreStackT*) hGreStackHandle;

  NW_ASSERT(thiz);
  NW_ASSERT(pUlpReq != NULL);

  NW_ENTER(thiz);

  switch(pUlpReq->apiType)
  {
    case NW_GRE_ULP_API_CREATE_TUNNEL_ENDPOINT:
      {
        NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Received create session req from ulp");
        rc = NwGreCreateTunnelEndPoint(thiz, 
            pUlpReq->apiInfo.createTunnelEndPointInfo.greKey,
            pUlpReq->apiInfo.createTunnelEndPointInfo.hUlpSession,
            &(pUlpReq->apiInfo.createTunnelEndPointInfo.hStackSession));
      }
      break;

    case NW_GRE_ULP_API_DESTROY_TUNNEL_ENDPOINT:
      {
        NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Received destroy session req from ulp");
        rc = nwGreDestroyTunnelEndPoint(thiz,  pUlpReq);
      }
      break;

    case NW_GRE_ULP_API_INITIAL_REQ:
      {
        NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Received initial req from ulp");
        rc = nwGreInitialReq(thiz, pUlpReq);
      }
      break;

    case NW_GRE_ULP_API_SEND_TPDU:
      {
        NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Received send tpdu req from ulp");
        rc = nwGreSendto(thiz,  pUlpReq);
      }
      break;

    default:
      NW_LOG(thiz, NW_LOG_LEVEL_ERRO, "Unsupported API received from ulp");
      rc = NW_FAILURE;
      break;
  }

  NW_LEAVE(thiz);

  return rc;
}

/*---------------------------------------------------------------------------
 * Process Timer timeout Request from Timer Manager
 *--------------------------------------------------------------------------*/

NwRcT
nwGreProcessTimeout(void* timeoutInfo)
{
  NwRcT rc;
  NwGreStackT* thiz;

  NW_ASSERT(timeoutInfo != NULL);

  thiz = (NwGreStackT*) (((NwGreTimeoutInfoT*) timeoutInfo)->hStack);

  NW_ASSERT(thiz != NULL);

  NW_ENTER(thiz);
  NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Received timeout event from ULP with timeoutInfo %x!", timeoutInfo);

  rc = (((NwGreTimeoutInfoT*) timeoutInfo)->timeoutCallbackFunc) (timeoutInfo);

  NW_LEAVE(thiz);

  return rc;
}

#ifdef __cplusplus
}
#endif

/*--------------------------------------------------------------------------*
 *                      E N D     O F    F I L E                            *
 *--------------------------------------------------------------------------*/


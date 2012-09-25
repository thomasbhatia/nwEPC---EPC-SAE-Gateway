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
#include <string.h>

#include "NwTypes.h"
#include "NwUtils.h"
#include "NwGreLog.h"
#include "NwGre.h"
#include "NwGrePrivate.h"
#include "NwGreTrxn.h"

/*--------------------------------------------------------------------------*
 *                 P R I V A T E  D E C L A R A T I O N S                   *
 *--------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

static NwGreTrxnT* gpGreTrxnPool = NULL;
/*--------------------------------------------------------------------------*
 *                   P R I V A T E      F U N C T I O N S                   *
 *--------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * Send msg retransmission to peer via data request to UDP Entity
 *--------------------------------------------------------------------------*/

static NwRcT
nwGreTrxnSendMsgRetransmission(NwGreTrxnT* thiz)
{
  NwRcT rc;

  NW_ASSERT(thiz);
  NW_ASSERT(thiz->pMsg);

  rc = thiz->pStack->udp.udpDataReqCallback(thiz->pStack->udp.hUdp, 
      thiz->pMsg->msgBuf, 
      thiz->pMsg->msgLen, 
      thiz->peerIp,
      thiz->peerPort);

  return rc;
}

static NwRcT
nwGreTrxnPeerRspTimeout(void* arg)
{
  NwRcT rc = NW_OK;
  NwGreTrxnT* thiz;
  NwGreStackT* pStack;
  NwGreTimeoutInfoT *timeoutInfo = arg;

  thiz = ((NwGreTrxnT*)timeoutInfo->timeoutArg);
  pStack = thiz->pStack;

  NW_ASSERT(pStack);

  NW_LOG(pStack, NW_LOG_LEVEL_WARN, "T3 timer expired for transaction 0x%X", thiz);

  rc = nwGreTrxnSendMsgRetransmission(thiz);

  if(thiz->maxRetries)
  {
    rc = pStack->tmrMgr.tmrStartCallback(pStack->tmrMgr.tmrMgrHandle, thiz->t3Timer, 0, NW_GRE_TMR_TYPE_ONE_SHOT, (void*)timeoutInfo, &thiz->hRspTmr);
    thiz->maxRetries--;
  }
  else
  {
    NwGreUlpApiT ulpApi;
    ulpApi.apiType                      = NW_GRE_ULP_API_RSP_FAILURE;
    ulpApi.apiInfo.recvMsgInfo.msgType  = nwGreMsgGetMsgType(thiz->pMsg);
    ulpApi.apiInfo.recvMsgInfo.hUlpTrxn = thiz->hUlpTrxn;
    ulpApi.apiInfo.recvMsgInfo.peerIp   = thiz->peerIp;
    ulpApi.apiInfo.recvMsgInfo.peerPort = thiz->peerPort;
    thiz->hRspTmr = 0;

    rc = nwGreTrxnDelete(&thiz);
    NW_ASSERT(NW_OK == rc);

    rc = pStack->ulp.ulpReqCallback(pStack->ulp.hUlp, &ulpApi);
    NW_ASSERT(NW_OK == rc);
  }
  return rc;
}

/**
  Send timer start request to TmrMgr Entity.

  @param[in] thiz : Pointer to transaction
  @param[in] timeoutCallbackFunc : Timeout handler callback function.
  @return NW_OK on success.
 */

static NwRcT
nwGreTrxnStartPeerRspTimer(NwGreTrxnT* thiz, NwRcT (*timeoutCallbackFunc)(void*))
{
  NwRcT rc;
  NwGreTimeoutInfoT *timeoutInfo;

  NW_ASSERT(thiz->pStack->tmrMgr.tmrStartCallback != NULL);

  timeoutInfo                           = &thiz->peerRspTimeoutInfo;
  timeoutInfo->timeoutArg               = thiz;
  timeoutInfo->timeoutCallbackFunc      = timeoutCallbackFunc;
  timeoutInfo->hStack                   = (NwGreStackHandleT)thiz->pStack;

  rc = thiz->pStack->tmrMgr.tmrStartCallback(thiz->pStack->tmrMgr.tmrMgrHandle, thiz->t3Timer, 0, NW_GRE_TMR_TYPE_ONE_SHOT, (void*)timeoutInfo, &thiz->hRspTmr);

  return rc;
}

/**
  Send timer stop request to TmrMgr Entity.

  @param[in] thiz : Pointer to transaction
  @return NW_OK on success.
 */

static NwRcT
nwGreTrxnStopPeerRspTimer(NwGreTrxnT* thiz)
{
  NwRcT rc;

  NW_ASSERT(thiz->pStack->tmrMgr.tmrStopCallback != NULL);

  rc = thiz->pStack->tmrMgr.tmrStopCallback(thiz->pStack->tmrMgr.tmrMgrHandle, thiz->hRspTmr);

  thiz->hRspTmr = 0;

  return rc;
}

/*--------------------------------------------------------------------------*
 *                      P U B L I C    F U N C T I O N S                    *
 *--------------------------------------------------------------------------*/

/**
 * Constructor
 *
 * @param[in] thiz : Pointer to stack  
 * @param[out] ppTrxn : Pointer to pointer to Trxn object.
 * @return NW_OK on success.
 */
NwRcT
nwGreTrxnNew( NW_IN  NwGreStackT* thiz,  
                 NW_OUT NwGreTrxnT **ppTrxn)
{
  NwRcT rc = NW_OK;

  NwGreTrxnT *pTrxn;

  NW_ASSERT(thiz);

  if(gpGreTrxnPool)
  {
    pTrxn = gpGreTrxnPool;
    gpGreTrxnPool = gpGreTrxnPool->next;
  }
  else
  {
    NW_GRE_MALLOC(thiz, sizeof(NwGreTrxnT), pTrxn, NwGreTrxnT*);
  }

  if (pTrxn)
  {
    pTrxn->maxRetries   = 2;
    pTrxn->pStack       = thiz;
    pTrxn->t3Timer      = 2;
    pTrxn->seqNum       = thiz->seq;

    /* Increment sequence number */
    thiz->seq++;
    if(thiz->seq == 0x800000)
      thiz->seq = 0;

  }
  else
  {
    rc = NW_FAILURE;
  }

  NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Created transaction 0x%X", pTrxn);

  *ppTrxn = pTrxn;

  return rc;
}

/**
 * Overloaded Constructor
 *
 * @param[in] thiz : Pointer to stack. 
 * @param[in] seqNum : Sequence number for this transaction. 
 * @param[out] ppTrxn : Pointer to pointer to Trxn object.
 * @return NW_OK on success.
 */
NwRcT
nwGreTrxnWithSeqNew( NW_IN  NwGreStackT* thiz,  
                        NW_IN  NwU32T seqNum,
                        NW_OUT NwGreTrxnT **ppTrxn)
{
  NwRcT rc = NW_OK;
  NwGreTrxnT *pTrxn;

  NW_ASSERT(thiz);

  if(gpGreTrxnPool)
  {
    pTrxn = gpGreTrxnPool;
    gpGreTrxnPool = gpGreTrxnPool->next;
  }
  else
  {
    NW_GRE_MALLOC(thiz, sizeof(NwGreTrxnT), pTrxn, NwGreTrxnT*);
  }


  if (pTrxn)
  {
    pTrxn->maxRetries   = 2;
    pTrxn->pStack       = thiz;
    pTrxn->t3Timer      = 2;
    pTrxn->seqNum       = seqNum;
    pTrxn->pMsg         = NULL;
  }
  else
  {
    rc = NW_FAILURE;
  }

  NW_LOG(thiz, NW_LOG_LEVEL_DEBG, "Created transaction 0x%X", pTrxn);

  *ppTrxn = pTrxn;

  return rc;
}

/**
 * Destructor
 *
 * @param[out] pthiz : Pointer to pointer to Trxn object.
 * @return NW_OK on success.
 */
NwRcT
nwGreTrxnDelete( NW_INOUT NwGreTrxnT **pthiz)
{
  NwRcT rc = NW_OK;
  NwGreStackT* pStack;
  NwGreTrxnT *thiz = *pthiz;

  pStack = thiz->pStack;

  if(thiz->hRspTmr)
  {
    rc = nwGreTrxnStopPeerRspTimer(thiz);
    NW_ASSERT(NW_OK == rc);
  }

  if(thiz->pMsg)
  {
    rc = nwGreMsgDelete((NwGreStackHandleT)pStack, (NwGreMsgHandleT)thiz->pMsg);
    NW_ASSERT(NW_OK == rc);
  }

  thiz->next = gpGreTrxnPool;
  gpGreTrxnPool = thiz;

  NW_LOG(pStack, NW_LOG_LEVEL_DEBG, "Purged transaction 0x%X", thiz);

  *pthiz = NULL;
  return rc;
}


#if 1
/**
 * Send msg to peer via data request to UDP Entity
 *
 * @param[in] thiz : Pointer to stack. 
 * @param[in] pTrxn : Pointer to Trxn object.
 * @param[in] peerIp : Peer Ip address.
 * @param[in] peerPort : Peer Ip port.
 * @param[in] pMsg : Message to be sent.
 * @return NW_OK on success.
 */
NwRcT
nwGreTrxnCreateAndSendMsg( NW_IN  NwGreStackT* thiz,
                         NW_IN  NwGreTrxnT *pTrxn,
                         NW_IN  NwU32T peerIp,
                         NW_IN  NwU32T peerPort,
                         NW_IN  NwGreMsgT *pMsg)
{
  NwRcT rc;
  NwU8T* msgHdr;

  NW_ASSERT(thiz);
  NW_ASSERT(pMsg);

  msgHdr = pMsg->msgBuf;
  NW_ASSERT(msgHdr != NULL);

  *(msgHdr++)         = (pMsg->version << 5)            | 
                        (pMsg->protocolType << 4)       | 
                        (pMsg->csumPresent << 2)         | 
                        (pMsg->keyPresent << 1)         | 
                        (pMsg->seqNumPresent);

  *(msgHdr++)         = (pMsg->msgType);
  *((NwU16T*) msgHdr) = htons(pMsg->msgLen);
  msgHdr += 2;

  *((NwU32T*) msgHdr) = htonl(pMsg->greKey);
  msgHdr += 4;

  if(pMsg->keyPresent | pMsg->csumPresent | pMsg->seqNumPresent)
  {
    if(pMsg->keyPresent)
    {
      *((NwU16T*) msgHdr) = htons((pTrxn ? pTrxn->seqNum : pMsg->seqNum));
    }
    else
    {
      *((NwU16T*) msgHdr) = 0x0000;
    }
    msgHdr += 2; 

    if(pMsg->seqNumPresent)
    {
      *((NwU8T*) msgHdr) = pMsg->seqNumPresent;
    }
    else
    {
      *((NwU8T*) msgHdr) = 0x00;
    }
    msgHdr++; 

    if(pMsg->csumPresent)
    {
      *((NwU8T*) msgHdr) = pMsg->csumPresent;
    }
    else
    {
      *((NwU8T*) msgHdr) = 0x00;
    }
    msgHdr++; 
  }

  NW_ASSERT(thiz->udp.udpDataReqCallback != NULL);

  rc = thiz->udp.udpDataReqCallback(thiz->udp.hUdp,
      pMsg->msgBuf,
      pMsg->msgLen,
      peerIp,
      peerPort);

  /* Save the message for retransmission */
  if(NW_OK == rc && pTrxn)
  {
    pTrxn->pMsg         = pMsg;
    pTrxn->peerIp       = peerIp;
    pTrxn->peerPort     = peerPort;

    rc = nwGreTrxnStartPeerRspTimer(pTrxn, nwGreTrxnPeerRspTimeout);
    NW_ASSERT(NW_OK == rc);
  }

  return rc;
}
#endif



#ifdef __cplusplus
}
#endif

/*--------------------------------------------------------------------------*
 *                          E N D   O F   F I L E                           * 
 *--------------------------------------------------------------------------*/


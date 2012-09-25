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
#include "NwGreMsg.h"

#define NW_GRE_EPC_SPECIFIC_HEADER_SIZE                             (12)   /**< Size of GRE EPC specific header */

#ifdef __cplusplus
extern "C" {
#endif

static NwGreMsgT* gpGreMsgPool = NULL;

NwRcT
nwGreMsgNew( NW_IN NwGreStackHandleT hGreStackHandle,
                NW_IN NwU8T     keyPresent,
                NW_IN NwU8T     seqNumPresent,
                NW_IN NwU8T     csumPresent,
                NW_IN NwU8T     msgType,
                NW_IN NwU8T     key,
                NW_IN NwU16T    seqNum,
                NW_IN NwU8T     npduNum,
                NW_IN NwU8T     nextExtHeader,
                NW_OUT NwGreMsgHandleT *phMsg)
{
  NwGreStackT* pStack = (NwGreStackT*) hGreStackHandle;
  NwGreMsgT *pMsg;

  NW_ASSERT(pStack);

  if(gpGreMsgPool)
  {
    pMsg = gpGreMsgPool;
    gpGreMsgPool = gpGreMsgPool->next;
  }
  else
  {
    NW_GRE_MALLOC(pStack, sizeof(NwGreMsgT), pMsg, NwGreMsgT*);
  }

  if(pMsg)
  {
    pMsg->version       = NW_GRE_VERSION;
    pMsg->protocolType  = NW_PROTOCOL_TYPE_IPv4;
    pMsg->csumPresent   = csumPresent;
    pMsg->keyPresent    = keyPresent;
    pMsg->seqNumPresent = seqNumPresent;
    pMsg->msgType       = msgType;

    if(keyPresent)
      pMsg->greKey      = key;
    if(seqNumPresent)
      pMsg->seqNum      = seqNum;

    pMsg->msgLen        = 4 + 
                          (pMsg->csumPresent ? 4 : 0 ) + 
                          (pMsg->keyPresent ? 4 : 0 ) +  
                          (pMsg->seqNumPresent ? 4 : 0 );  


    *phMsg = (NwGreMsgHandleT) pMsg;
    return NW_OK;
  }

  return NW_FAILURE;
}

NwRcT
nwGreGpduMsgNew( NW_IN NwGreStackHandleT hGreStackHandle,
                NW_IN NwU8T     csumPresent,
                NW_IN NwU8T     keyPresent,
                NW_IN NwU8T     seqNumPresent,
                NW_IN NwU32T    key,
                NW_IN NwU16T    seqNum,
                NW_IN NwU8T*    tpdu,
                NW_IN NwU16T    tpduLength,
                NW_OUT NwGreMsgHandleT *phMsg)
{
  NwGreStackT* pStack = (NwGreStackT*) hGreStackHandle;
  NwGreMsgT *pMsg;

  NW_ASSERT(pStack);

  if(gpGreMsgPool)
  {
    pMsg = gpGreMsgPool;
    gpGreMsgPool = gpGreMsgPool->next;
  }
  else
  {
    NW_GRE_MALLOC(pStack, sizeof(NwGreMsgT), pMsg, NwGreMsgT*);
  }

  if(pMsg)
  {
    pMsg->version       = NW_GRE_VERSION;
    pMsg->protocolType  = NW_PROTOCOL_TYPE_IPv4;
    pMsg->csumPresent   = csumPresent;
    pMsg->keyPresent    = keyPresent;
    pMsg->seqNumPresent = seqNumPresent;

    if(keyPresent)
      pMsg->greKey      = key;
    if(seqNumPresent)
      pMsg->seqNum      = seqNum;

    pMsg->msgLen        = 4 + 
      (pMsg->csumPresent ? 4 : 0 ) + 
      (pMsg->keyPresent ? 4 : 0 ) +  
      (pMsg->seqNumPresent ? 4 : 0 );  

    memcpy(pMsg->msgBuf + pMsg->msgLen, tpdu, tpduLength);
    pMsg->msgLen        += tpduLength;

    *phMsg = (NwGreMsgHandleT) pMsg;
    return NW_OK;

  }

  return NW_FAILURE;
}

NwRcT
nwGreMsgFromBufferNew( NW_IN NwGreStackHandleT hGreStackHandle,
                         NW_IN NwU8T* pBuf,
                         NW_IN NwU32T bufLen,
                         NW_OUT NwGreMsgHandleT *phMsg)
{
  NwGreStackT* pStack = (NwGreStackT*) hGreStackHandle;
  NwGreMsgT *pMsg;

  NW_ASSERT(pStack);

  if(gpGreMsgPool)
  {
    pMsg = gpGreMsgPool;
    gpGreMsgPool = gpGreMsgPool->next;
  }
  else
  {
    NW_GRE_MALLOC(pStack, sizeof(NwGreMsgT), pMsg, NwGreMsgT*);
  }

  if(pMsg)
  {
    *phMsg = (NwGreMsgHandleT) pMsg;
    memcpy(pMsg->msgBuf, pBuf, bufLen);
    pMsg->msgLen = bufLen;

    pMsg->csumPresent   = ((*pBuf) & 0x80) >> 7;
    pMsg->keyPresent    = ((*pBuf) & 0x20) >> 5;
    pMsg->seqNumPresent = ((*pBuf) & 0x10) >> 4;
    pBuf++;

    pMsg->version       = ((*pBuf) & 0x03);
    pBuf++;

    pMsg->protocolType  = ntohs(*((NwU16T*)pBuf));
    pBuf += 2;

    if(pMsg->keyPresent)
    {
      // TODO: Validate CSum.
    }

    if(pMsg->keyPresent)
    {
      pMsg->greKey      = ntohl(*((NwU32T*)pBuf));
      pBuf += 4;
    }

    if(pMsg->seqNumPresent)
    {
      pMsg->seqNum      = ntohl(*((NwU32T*)pBuf));
      pBuf += 4;
    }

    return NW_OK;
  }
  return NW_FAILURE;
}

NwRcT
nwGreMsgDelete( NW_IN NwGreStackHandleT hGreStackHandle,
                   NW_IN NwGreMsgHandleT hMsg)
{
  ((NwGreMsgT*)hMsg)->next = gpGreMsgPool;
  gpGreMsgPool = (NwGreMsgT*)hMsg;
  return NW_OK;
}

 /**
  * Set TEID for gre message.
  *
  * @param[in] hMsg : Message handle.
  * @param[in] teid: TEID value.
  */

NwRcT
nwGreMsgSetTeid(NW_IN NwGreMsgHandleT hMsg, NwU32T teid)
{
  NwGreMsgT *thiz = (NwGreMsgT*) hMsg;
  thiz->greKey = teid; 
  return NW_OK;
}

 /**
  * Set sequence for gre message.
  *
  * @param[in] hMsg : Message handle.
  * @param[in] seqNum: Flag boolean value.
  */

NwRcT
nwGreMsgSetSeqNumber(NW_IN NwGreMsgHandleT hMsg, NwU32T seqNum)
{
  NwGreMsgT *thiz = (NwGreMsgT*) hMsg;
  thiz->seqNum = seqNum; 
  return NW_OK;
}

 /**
  * Get TEID present for gre message.
  *
  * @param[in] hMsg : Message handle.
  */

NwU32T
nwGreMsgGetTeid(NW_IN NwGreMsgHandleT hMsg)
{
  NwGreMsgT *thiz = (NwGreMsgT*) hMsg;
  return (thiz->greKey); 
}


 /**
  * Get sequence number for gre message.
  *
  * @param[in] hMsg : Message handle.
  */

NwU32T
nwGreMsgGetSeqNumber(NW_IN NwGreMsgHandleT hMsg)
{
  NwGreMsgT *thiz = (NwGreMsgT*) hMsg;
  return (thiz->seqNum);
}

 /**
  * Get msg type for gre message.
  *
  * @param[in] hMsg : Message handle.
  */

NwU32T
nwGreMsgGetMsgType(NW_IN NwGreMsgHandleT hMsg)
{
  NwGreMsgT *thiz = (NwGreMsgT*) hMsg;
  return (thiz->msgType);
}

 /**
  * Get tpdu for gre message.
  *
  * @param[in] hMsg : Message handle.
  */

NwU32T
nwGreMsgGetTpdu(NW_IN NwGreMsgHandleT hMsg, NwU8T* pTpduBuf, NwU32T* pTpduLength)
{
  NwGreMsgT *thiz = (NwGreMsgT*) hMsg;
  NwU8T headerLength = 4 + 
    (thiz->csumPresent ? 4 : 0 ) + 
    (thiz->keyPresent ? 4 : 0 ) +  
    (thiz->seqNumPresent ? 4 : 0 );  

  *pTpduLength = thiz->msgLen - headerLength;
  memcpy(pTpduBuf, thiz->msgBuf + headerLength, *pTpduLength);
  return NW_OK;
}

NwRcT
nwGreMsgAddIeTV1(NW_IN NwGreMsgHandleT hMsg, 
              NW_IN NwU8T       type,
              NW_IN NwU8T       instance,
              NW_IN NwU8T       value)
{
  NwGreMsgT *pMsg = (NwGreMsgT*) hMsg;
  NwGreIeTv1T *pIe;

  pIe = (NwGreIeTv1T*) (pMsg->msgBuf + pMsg->msgLen);

  pIe->t        = type;
  pIe->l        = htons(0x0001);
  pIe->i        = instance & 0x00ff;
  pIe->v        = value;

  pMsg->msgLen += sizeof(NwGreIeTv1T);

  return NW_OK;
}

NwRcT
nwGreMsgAddIeTV2(NW_IN NwGreMsgHandleT hMsg, 
              NW_IN NwU8T       type,
              NW_IN NwU16T      length,
              NW_IN NwU8T       instance,
              NW_IN NwU16T      value)
{
  NwGreMsgT *pMsg = (NwGreMsgT*) hMsg;
  NwGreIeTv2T *pIe;

  pIe = (NwGreIeTv2T*) (pMsg->msgBuf + pMsg->msgLen);

  pIe->t        = type;
  pIe->l        = htons(0x0001);
  pIe->i        = instance & 0x00ff;
  pIe->v        = htons(value);

  pMsg->msgLen += sizeof(NwGreIeTv2T);

  return NW_OK;
}

NwRcT
nwGreMsgAddIeTV4(NW_IN NwGreMsgHandleT hMsg, 
              NW_IN NwU8T       type,
              NW_IN NwU16T      length,
              NW_IN NwU8T       instance,
              NW_IN NwU32T      value)
{
  NwGreMsgT *pMsg = (NwGreMsgT*) hMsg;
  NwGreIeTv4T *pIe;

  pIe = (NwGreIeTv4T*) (pMsg->msgBuf + pMsg->msgLen);

  pIe->t        = type;
  pIe->l        = htons(length);
  pIe->i        = instance & 0x00ff;
  pIe->v        = htonl(value);

  pMsg->msgLen += sizeof(NwGreIeTv4T);

  return NW_OK;
}

NwRcT
nwGreMsgAddIe(NW_IN NwGreMsgHandleT hMsg, 
              NW_IN NwU8T       type,
              NW_IN NwU16T      length,
              NW_IN NwU8T       instance,
              NW_IN NwU8T*      pVal)
{
  NwGreMsgT *pMsg = (NwGreMsgT*) hMsg;
  NwGreIeTlvT *pIe;

  pIe = (NwGreIeTlvT*) (pMsg->msgBuf + pMsg->msgLen);

  pIe->t        = type;
  pIe->l        = htons(length);
  pIe->i        = instance & 0x00ff;

  memcpy(pIe + 4, pVal, length);
  pMsg->msgLen += (4 + length);

  return NW_OK;
}

NwRcT
nwGreMsgHexDump(NwGreMsgHandleT hMsg, FILE* fp)
{

  NwGreMsgT* pMsg = (NwGreMsgT*) hMsg;
  NwU8T* data = pMsg->msgBuf;
  NwU32T size = pMsg->msgLen;

  unsigned char *p = (unsigned char*)data;
  unsigned char c;
  int n;
  char bytestr[4] = {0};
  char addrstr[10] = {0};
  char hexstr[ 16*3 + 5] = {0};
  char charstr[16*1 + 5] = {0};
  fprintf((FILE*)fp, "\n");
  for(n=1;n<=size;n++) {
    if (n%16 == 1) {
      /* store address for this line */
      snprintf(addrstr, sizeof(addrstr), "%.4x",
          ((unsigned int)p-(unsigned int)data) );
    }

    c = *p;
    if (isalnum(c) == 0) {
      c = '.';
    }

    /* store hex str (for left side) */
    snprintf(bytestr, sizeof(bytestr), "%02X ", *p);
    strncat(hexstr, bytestr, sizeof(hexstr)-strlen(hexstr)-1);

    /* store char str (for right side) */
    snprintf(bytestr, sizeof(bytestr), "%c", c);
    strncat(charstr, bytestr, sizeof(charstr)-strlen(charstr)-1);
    if(n%16 == 0) {
      /* line completed */
      fprintf((FILE*)fp, "[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
      hexstr[0] = 0;
      charstr[0] = 0;
    } else if(n%8 == 0) {
      /* half line: add whitespaces */
      strncat(hexstr, "  ", sizeof(hexstr)-strlen(hexstr)-1);
      strncat(charstr, " ", sizeof(charstr)-strlen(charstr)-1);
    }
    p++; /* next byte */
  }

  if (strlen(hexstr) > 0) {
    /* print rest of buffer if not empty */
    fprintf((FILE*)fp, "[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);

  }
  fprintf((FILE*)fp, "\n");

  return NW_OK;
}

#ifdef __cplusplus
}
#endif



/*--------------------------------------------------------------------------*
 *                          E N D   O F   F I L E                           * 
 *--------------------------------------------------------------------------*/


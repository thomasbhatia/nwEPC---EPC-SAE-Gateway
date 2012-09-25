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
#include <string.h>

#include "NwTypes.h"
#include "NwUtils.h"
#include "NwIpv4Log.h"
#include "NwIpv4.h"
#include "NwIpv4Private.h"
#include "NwIpv4Msg.h"

#ifdef __cplusplus
extern "C" {
#endif

static NwIpv4MsgT *gpIpv4MsgPool = NULL;

NwIpv4RcT
nwIpv4MsgFromBufferNew( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
                         NW_IN NwU8T* pBuf,
                         NW_IN NwU32T bufLen,
                         NW_OUT NwIpv4MsgHandleT *phMsg)
{
  NwIpv4MsgT *pMsg;

  if(gpIpv4MsgPool)
  {
    pMsg = gpIpv4MsgPool;
    gpIpv4MsgPool = gpIpv4MsgPool->next;
  }
  else
  {
    NW_IPv4_MALLOC(((NwIpv4StackT*)hIpv4StackHandle), sizeof(NwIpv4MsgT), pMsg, NwIpv4MsgT*);
  }

  if(pMsg)
  {
    *phMsg = (NwIpv4MsgHandleT) pMsg;
    memcpy(pMsg->msgBuf, pBuf, bufLen);
    pMsg->msgLen = bufLen;

    return NW_IPv4_OK;
  }
  return NW_IPv4_FAILURE;
}

NwIpv4RcT
nwIpv4MsgDelete( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
                   NW_IN NwIpv4MsgHandleT hMsg)
{
  NwIpv4MsgT *pMsg = (NwIpv4MsgT*) hMsg;
  pMsg->next    = gpIpv4MsgPool;
  gpIpv4MsgPool = pMsg;
  return NW_IPv4_OK;
}

NwU8T*
nwIpv4MsgGetBufHandle( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
                        NW_IN NwIpv4MsgHandleT hMsg)
{
  return (((NwIpv4MsgT*)hMsg)->msgBuf);
}

NwU32T
nwIpv4MsgGetLength( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
                    NW_IN NwIpv4MsgHandleT hMsg)
{
  return (((NwIpv4MsgT*)hMsg)->msgLen);
}

NwIpv4RcT
nwIpv4MsgHexDump(NwIpv4MsgHandleT hMsg, FILE* fp)
{

  NwIpv4MsgT* pMsg = (NwIpv4MsgT*) hMsg;
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

  return NW_IPv4_OK;
}

#ifdef __cplusplus
}
#endif



/*--------------------------------------------------------------------------*
 *                          E N D   O F   F I L E                           * 
 *--------------------------------------------------------------------------*/


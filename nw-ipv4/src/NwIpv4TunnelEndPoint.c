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

#include "NwTypes.h"
#include "NwUtils.h"
#include "NwIpv4Error.h"
#include "NwIpv4Private.h"
#include "NwIpv4Msg.h"
#include "NwIpv4TunnelEndPoint.h"
#include "NwIpv4.h"
#include "NwIpv4Log.h"

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------*
 *                  P R I V A T E   D E C L A R A T I O N S                 *
 *--------------------------------------------------------------------------*/

static NwIpv4TunnelEndPointT *gpIpv4TunnelEndPointPool = NULL;

/*--------------------------------------------------------------------------*
 *                     P U B L I C   F U N C T I O N S                      *
 *--------------------------------------------------------------------------*/

/**
  Constructor

  @return Pointer to Session on success, NULL n failure.
 */


NwIpv4TunnelEndPointT*
nwIpv4TunnelEndPointNew(struct NwIpv4Stack *pStack)
{
  NwIpv4TunnelEndPointT* thiz;
  if(gpIpv4TunnelEndPointPool)
  {
    thiz = gpIpv4TunnelEndPointPool;
    gpIpv4TunnelEndPointPool = gpIpv4TunnelEndPointPool->next;
  }
  else
  {
    NW_IPv4_MALLOC(pStack, sizeof(NwIpv4TunnelEndPointT), thiz, NwIpv4TunnelEndPointT*);
  }
  return thiz;
}

/**
  Destructor

  @param[in] thiz: Pointer to session
  @return NW_IPv4_OK on success.
 */

NwIpv4RcT
nwIpv4TunnelEndPointDestroy(struct NwIpv4Stack *pStack, NwIpv4TunnelEndPointT* thiz)
{
  thiz->next = gpIpv4TunnelEndPointPool;
  gpIpv4TunnelEndPointPool = thiz;
  return NW_IPv4_OK;
}

/**
  Send a API msg to ULP for a Session.

  @param[in] thiz: Pointer to session
  @return NW_IPv4_OK on success.
 */

NwIpv4RcT
nwIpv4SessionSendMsgApiToUlpEntity(NwIpv4TunnelEndPointT* thiz,
                                   NwU8T* pIpPdu,
                                   NwU32T ipPduLen)
{
  NwIpv4RcT rc = NW_IPv4_OK;
  NwIpv4UlpApiT api;

  api.apiType                         = NW_IPv4_ULP_API_RECV_TPDU;
  api.apiInfo.recvMsgInfo.hUlpSession = thiz->hUlpSession;
  rc = nwIpv4MsgFromBufferNew((NwIpv4StackHandleT)thiz, pIpPdu, ipPduLen, &api.apiInfo.recvMsgInfo.hMsg); 

  NW_ASSERT(thiz->pStack->ulp.ulpReqCallback != NULL);

  thiz->pStack->ulp.ulpReqCallback(thiz->pStack->ulp.hUlp, &api);

  return rc;
}

#ifdef __cplusplus
}
#endif

/*--------------------------------------------------------------------------*
 *                      E N D     O F    F I L E                            *
 *--------------------------------------------------------------------------*/


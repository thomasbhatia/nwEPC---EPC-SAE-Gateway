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
#include "NwGreMsg.h"
#include "NwGreTunnelEndPoint.h"
#include "NwGre.h"
#include "NwGreLog.h"

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------*
 *                  P R I V A T E   D E C L A R A T I O N S                 *
 *--------------------------------------------------------------------------*/

static NwGreTunnelEndPointT* gpGreTunnelEndPointPool = NULL;

/*--------------------------------------------------------------------------*
 *                     P U B L I C   F U N C T I O N S                      *
 *--------------------------------------------------------------------------*/

/**
  Constructor

  @param[in] msid: MSID 
  @param[in] peerAddr: Address of the peer.
  @return Pointer to Session on success, NULL n failure.
 */


NwGreTunnelEndPointT*
nwGreTunnelEndPointNew(struct NwGreStack *pStack)
{
  NwGreTunnelEndPointT* thiz;
  if(gpGreTunnelEndPointPool)
  {
    thiz = gpGreTunnelEndPointPool;
    gpGreTunnelEndPointPool = gpGreTunnelEndPointPool->next;
  }
  else
  {
    NW_GRE_MALLOC(pStack, sizeof(NwGreTunnelEndPointT), thiz, NwGreTunnelEndPointT*);
  }
  return thiz;
}

/**
  Destructor

  @param[in] thiz: Pointer to session
  @return NW_OK on success.
 */

NwRcT
nwGreTunnelEndPointDestroy(struct NwGreStack *pStack, NwGreTunnelEndPointT* thiz)
{
  thiz->next = gpGreTunnelEndPointPool;
  gpGreTunnelEndPointPool = thiz;
  return NW_OK;
}

/**
  Purge a Transaction for a Session.

  @param[in] thiz: Pointer to session
  @param[in,out] pTrxn: Pointer to the trxn.
  @return NW_OK on success.
 */

NwRcT
nwGreSessionSendMsgApiToUlpEntity(NwGreTunnelEndPointT* thiz,
    NwGreMsgT *pMsg)
{
  NwRcT rc = NW_OK;
  NwGreUlpApiT api;

  api.apiType                         = NW_GRE_ULP_API_RECV_TPDU;
  api.apiInfo.recvMsgInfo.hUlpSession = thiz->hUlpSession;
  api.apiInfo.recvMsgInfo.greKey      = thiz->greKey;
  api.apiInfo.recvMsgInfo.hMsg        = (NwGreMsgHandleT)pMsg;

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


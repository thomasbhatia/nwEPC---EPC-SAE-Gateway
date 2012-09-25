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

#include "NwTypes.h"
#include "NwUtils.h"
#include "NwSdpError.h"
#include "NwSdpPrivate.h"
#include "NwSdpFlowContext.h"
#include "NwSdp.h"
#include "NwSdpLog.h"

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------*
 *                  P R I V A T E   D E C L A R A T I O N S                 *
 *--------------------------------------------------------------------------*/

static NwSdpFlowContextT *gpSdpFlowContextPool = NULL;

/*--------------------------------------------------------------------------*
 *                     P U B L I C   F U N C T I O N S                      *
 *--------------------------------------------------------------------------*/

/**
  Constructor

  @return Pointer to Session on success, NULL on failure.
 */


NwSdpFlowContextT*
nwSdpFlowContextNew(NwSdpT* pSdp)
{
  NwSdpFlowContextT* thiz;

  if(gpSdpFlowContextPool)
  {
    thiz = gpSdpFlowContextPool;
    gpSdpFlowContextPool = gpSdpFlowContextPool->next;
  }
  else
  {
    NW_SDP_MALLOC(pSdp, sizeof(NwSdpFlowContextT), thiz, NwSdpFlowContextT*);
  }

  return thiz;
}

/**
  Destructor

  @return NW_SDP_OK on success.
 */

NwSdpRcT
nwSdpFlowContextDelete(NwSdpT* pSdp, NwSdpFlowContextT* thiz)
{
  thiz->next = gpSdpFlowContextPool;
  gpSdpFlowContextPool = thiz;

  return NW_SDP_OK;
}

#ifdef __cplusplus
}
#endif

/*--------------------------------------------------------------------------*
 *                      E N D     O F    F I L E                            *
 *--------------------------------------------------------------------------*/


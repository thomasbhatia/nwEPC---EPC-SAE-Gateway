/*----------------------------------------------------------------------------*
 *                                                                            *
 *                                n w - e p c                                 * 
 *       L T E / S A E        S E R V I N G / P D N       G A T E W A Y       *
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
 * @file NwSaeGwUeFsm.c
 */

#include <stdio.h>
#include <assert.h>

#include "NwLog.h"
#include "NwMem.h"
#include "NwTypes.h"
#include "NwUtils.h"
#include "NwLogMgr.h"
#include "NwSaeGwUeLog.h"
#include "NwSaeGwUeState.h"
#include "NwSaeGwUeFsm.h"
#include "NwSaeGwUeStateInit.h"
#include "NwSaeGwUeStateSaeSessionCreated.h"
#include "NwSaeGwUeStateSgwSessionCreated.h"
#include "NwSaeGwUeStatePgwSessionCreated.h"
#include "NwSaeGwUeStateSaeSessionEstablished.h"
#include "NwSaeGwUeStateSgwSessionEstablished.h"
#include "NwSaeGwUeStateAwaitPgwCreateSessionRsp.h"
#include "NwSaeGwUeStateAwaitPgwDeleteSessionRsp.h"
#include "NwSaeGwUe.h"

#ifdef __cplusplus
extern "C" {
#endif

static NwSaeGwUeFsmT* gpSaeGwFsm = NULL;

#define NW_FSM_ENTER()
#define NW_FSM_LEAVE()

NwSaeGwUeFsmT*
NwSaeGwUeFsmNew()
{
  NwRcT rc;
  NwSaeGwUeFsmT* thiz;

  NW_FSM_ENTER();

  if(gpSaeGwFsm)
  {
    thiz = gpSaeGwFsm;
  }
  else
  {

    thiz = (NwSaeGwUeFsmT*) nwMemNew (sizeof(NwSaeGwUeFsmT));
    memset(thiz, 0x00, sizeof(NwSaeGwUeFsmT));

    /* Initialize states */
    thiz->pState[NW_SAE_GW_UE_STATE_INIT] = nwSaeGwStateInitNew(); 
    thiz->pState[NW_SAE_GW_UE_STATE_SAE_SESSION_CREATED] = nwSaeGwStateSaeSessionCreatedNew(); 
    thiz->pState[NW_SAE_GW_UE_STATE_SGW_SESSION_CREATED] = nwSaeGwStateSgwSessionCreatedNew(); 
    thiz->pState[NW_SAE_GW_UE_STATE_PGW_SESSION_CREATED] = nwSaeGwStatePgwSessionCreatedNew(); 
    thiz->pState[NW_SAE_GW_UE_STATE_SAE_SESSION_ESTABLISHED] = nwSaeGwStateSaeSessionEstablishedNew(); 
    thiz->pState[NW_SAE_GW_UE_STATE_SGW_SESSION_ESTABLISHED] = nwSaeGwStateSgwSessionEstablishedNew(); 
    thiz->pState[NW_SAE_GW_UE_STATE_WT_PGW_CREATE_SESSION_RSP] = nwSaeGwStateAwaitPgwCreateSessionRspNew(); 
    thiz->pState[NW_SAE_GW_UE_STATE_WT_PGW_DELETE_SESSION_RSP] = nwSaeGwStateAwaitPgwDeleteSessionRspNew(); 

    rc = nwSaeGwStateSetParentState(thiz->pState[NW_SAE_GW_UE_STATE_SGW_SESSION_CREATED], 
                                    thiz->pState[NW_SAE_GW_UE_STATE_SAE_SESSION_CREATED]);
    NW_ASSERT( NW_OK == rc );
    gpSaeGwFsm = thiz;
  }

  NW_FSM_LEAVE();
  return thiz;
}

NwRcT
NwSaeGwUeFsmDelete(NwSaeGwUeFsmT* thiz)
{
  NW_FSM_ENTER();
  NW_ASSERT(gpSaeGwFsm == thiz);
  nwMemDelete((void*)thiz);
  gpSaeGwFsm = NULL;
  NW_FSM_LEAVE();
  return NW_OK;
}

NwRcT
nwSaeGwUeFsmRun(NwSaeGwUeFsmT* thiz, NwSaeGwUeT* pUe, NwSaeGwUeEventInfoT* pEv, NwUeStateT* pUeState)
{
  NwRcT rc;
  NwSaeUeStateT *pState;

  NW_ASSERT(thiz);

  NW_FSM_ENTER();

  *pUeState = (pUe->state);

  pState = thiz->pState[pUe->state];

  if(pState)
  {
    rc = nwSaeGwStateHandleEvent(pState, pUe, pEv);
    if(*pUeState != pUe->state)
    {
      /* Exit the Current State */
      if(pState->exit)
        pState->exit(pUe, pEv);

      pState = thiz->pState[pUe->state];

      /* Enter the next State */
      if(pState && pState->enter)
        pState->enter(pUe, pEv);
    }
  }
  else
  {
    rc = nwSaeGwUeUnexpectedEvent(pUe, pEv);
  }

  *pUeState = (pUe->state);

  NW_FSM_LEAVE();
  return rc;
}

#ifdef __cplusplus
}
#endif


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
 * @file NwSaeGwUeState.h
 */

#ifndef __NW_SAE_GW_UE_STATE__
#define __NW_SAE_GW_UE_STATE__

#include <stdio.h>
#include <assert.h>

#include "NwLog.h"
#include "NwTypes.h"
#include "NwSaeGwUe.h"

#define NW_SAE_GW_UE_MAXIMUM_EVENT                     (NW_SAE_GW_UE_EVENT_END)

typedef NwRcT (*NwSaeGwUeStateEventHandlerT)(NwSaeGwUeT* pUe, NwSaeGwUeEventInfoT* pEv);
typedef NwRcT (*NwSaeGwUeStateEnterT)(NwSaeGwUeT* pUe, NwSaeGwUeEventInfoT* pEv);
typedef NwRcT (*NwSaeGwUeStateExitT)(NwSaeGwUeT* pUe, NwSaeGwUeEventInfoT* pEv);

typedef struct NwSaeUeState
{
  struct NwSaeUeState  *pParentState;
  NwSaeGwUeStateEnterT enter;
  NwSaeGwUeStateExitT  exit;
  NwSaeGwUeStateEventHandlerT eventHandler[NW_SAE_GW_UE_MAXIMUM_EVENT];
} NwSaeUeStateT;

#ifdef __cplusplus
extern "C" { 
#endif
NwSaeUeStateT*
nwSaeGwStateNew();

NwRcT
nwSaeGwStateDelete(NwSaeUeStateT* thiz);

NwRcT
nwSaeGwStateSetParentState(NwSaeUeStateT *thiz, NwSaeUeStateT *pParentState);

NwRcT
nwSaeGwStateSetEntryAction(NwSaeUeStateT *thiz, NwSaeGwUeStateEventHandlerT eventHandler);

NwRcT
nwSaeGwStateSetExitAction(NwSaeUeStateT *thiz, NwSaeGwUeStateEventHandlerT eventHandler);

NwRcT
nwSaeGwStateSetEventHandler(NwSaeUeStateT *thiz, NwSaeGwUeEventT event, NwSaeGwUeStateEventHandlerT eventHandler);

NwRcT
nwSaeGwStateHandleEvent(NwSaeUeStateT *thiz, NwSaeGwUeT* pUe, NwSaeGwUeEventInfoT* pEv);

#ifdef __cplusplus
}
#endif

#endif

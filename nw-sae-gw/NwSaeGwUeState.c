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
 * @file NwSaeGwUeState.c
 */

#include <stdio.h>
#include <assert.h>

#include "NwLog.h"
#include "NwMem.h"
#include "NwTypes.h"
#include "NwUtils.h"
#include "NwSaeGwUeState.h"

#ifdef __cplusplus
extern "C" {
#endif

NwSaeUeStateT*
nwSaeGwStateNew()
{
  NwSaeUeStateT* thiz = (NwSaeUeStateT*) nwMemNew (sizeof(NwSaeUeStateT));
  if(thiz)
  {
    memset(thiz, 0, sizeof(NwSaeUeStateT));
  }
  return thiz;
}

NwRcT
nwSaeGwStateDelete(NwSaeUeStateT* thiz)
{
  nwMemDelete((void*)thiz);
  return NW_OK;
}

NwRcT
nwSaeGwStateSetParentState(NwSaeUeStateT *thiz, NwSaeUeStateT *pParentState)
{
  if(thiz)
  {
    thiz->pParentState = pParentState;
    return NW_OK;
  }
  return NW_FAILURE;
}

NwRcT
nwSaeGwStateSetEntryAction(NwSaeUeStateT *thiz, NwSaeGwUeStateEventHandlerT eventHandler)
{
  if(thiz)
  {
    thiz->enter = eventHandler;
    return NW_OK;
  }
  return NW_FAILURE;
}

NwRcT
nwSaeGwStateSetExitAction(NwSaeUeStateT *thiz, NwSaeGwUeStateEventHandlerT eventHandler)
{
  if(thiz)
  {
    thiz->exit = eventHandler;
    return NW_OK;
  }
  return NW_FAILURE;
}

NwRcT
nwSaeGwStateSetEventHandler(NwSaeUeStateT *thiz, NwSaeGwUeEventT event, NwSaeGwUeStateEventHandlerT eventHandler)
{
  if(thiz)
  {
    NW_ASSERT(NW_SAE_GW_UE_MAXIMUM_EVENT > event);
    thiz->eventHandler[event] = eventHandler;
    return NW_OK;
  }
  return NW_FAILURE;
}

NwRcT
nwSaeGwStateHandleEvent(NwSaeUeStateT *thiz, NwSaeGwUeT* pUe, NwSaeGwUeEventInfoT* pEv)
{
  NwRcT rc;

  if(thiz)
  {
    NW_ASSERT(NW_SAE_GW_UE_MAXIMUM_EVENT > pEv->event);
    if(thiz->eventHandler[pEv->event])
    {
      rc = thiz->eventHandler[pEv->event](pUe, pEv);
    }
    else if(thiz->pParentState)
    {
      rc = nwSaeGwStateHandleEvent(thiz->pParentState, pUe, pEv);
    }
    else
    {
      rc = nwSaeGwUeUnexpectedEvent(pUe, pEv);
    }
  }
  else
  {
    rc = NW_FAILURE;
  }

  return rc;
}

#ifdef __cplusplus
}
#endif


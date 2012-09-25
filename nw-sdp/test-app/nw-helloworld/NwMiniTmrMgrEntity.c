/*----------------------------------------------------------------------------*
 *                                                                            *
 *         M I N I M A L I S T I C    T M R M G R     E N T I T Y             *
 *                                                                            *
 *                    Copyright (C) 2010 Amit Chawre.                         *
 *                                                                            *
 *----------------------------------------------------------------------------*/

/** 
 * @file NwMiniTmrMgrEntity.c
 * @brief This file ontains example of a minimalistic timer manager entity.
*/

#include <stdio.h>
#include <assert.h>
#include "NwEvt.h"
#include "NwSdp.h"
#include "NwMiniLogMgrEntity.h"
#include "NwMiniTmrMgrEntity.h"

#ifndef NW_ASSERT
#define NW_ASSERT assert
#endif 

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------
 * Private functions
 *--------------------------------------------------------------------------*/

static void
NW_TMR_CALLBACK(nwMiniTmrMgrHandleTimeout)
{
  NwSdpRcT rc;
  NwMiniTmrT *pTmr = (NwMiniTmrT*) arg;

  /*---------------------------------------------------------------------------
   *  Send Timeout Request to GTPv1u Stack Instance
   *--------------------------------------------------------------------------*/

  rc = nwSdpProcessTimeout(pTmr->timeoutArg);
  NW_ASSERT( rc == NW_SDP_OK );

  free(pTmr);

  return;
}

/*---------------------------------------------------------------------------
 * Public functions
 *--------------------------------------------------------------------------*/

NwSdpRcT nwTimerStart( NwSdpTimerMgrHandleT tmrMgrHandle,
    NwU32T timeoutSec,
    NwU32T timeoutUsec,
    NwU32T tmrType,
    void*  timeoutArg,
    NwSdpTimerHandleT* hTmr)
{
  NwSdpRcT rc = NW_SDP_OK;
  NwMiniTmrT *pTmr;
  struct timeval tv;

  NW_LOG(NW_LOG_LEVEL_INFO, "Received start timer request from stack with timer type %u, arg %x, for %u sec and %u usec", tmrType, timeoutArg, timeoutSec, timeoutUsec);
#ifdef __WITH_LIBEVENT__ 
  pTmr = (NwMiniTmrT*) malloc(sizeof(NwMiniTmrT));

  /* set the timevalues*/
  timerclear(&tv);
  tv.tv_sec     = timeoutSec;
  tv.tv_usec    = timeoutUsec;

  pTmr->timeoutArg = timeoutArg;
  evtimer_set(&pTmr->ev, nwMiniTmrMgrHandleTimeout, pTmr);

  /*add event*/

  event_add(&(pTmr->ev), &tv);
#else
#warning "Timer library not defined!"
#endif

  *hTmr = (NwSdpTimerHandleT)pTmr;

  return rc;
}

NwSdpRcT nwTimerStop( NwSdpTimerMgrHandleT tmrMgrHandle,
    NwSdpTimerHandleT hTmr)
{
  NW_LOG(NW_LOG_LEVEL_INFO, "Received stop timer request from stack for timer handle %u", hTmr);
#ifdef __WITH_LIBEVENT__ 
  evtimer_del(&(((NwMiniTmrT*)hTmr)->ev));
  free((void*)hTmr);
#else
#warning "Timer library not defined!"
#endif
  return NW_SDP_OK;
}

#ifdef __cplusplus
}
#endif

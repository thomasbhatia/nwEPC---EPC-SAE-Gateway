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
#include "NwIpv4.h"
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
  NwIpv4RcT rc;
#ifdef __WITH_LIBEVENT__
  NwMiniTmrT *pTmr = (NwMiniTmrT*) arg;

  /*---------------------------------------------------------------------------
   *  Send Timeout Request to IPv4 Stack Instance
   *--------------------------------------------------------------------------*/

  rc = nwIpv4ProcessTimeout(pTmr->timeoutArg);
  NW_ASSERT( rc == NW_IPv4_OK );
  free(pTmr);
#else
#warning "Timer library not defined!"
#endif


  return;
}

/*---------------------------------------------------------------------------
 * Public functions
 *--------------------------------------------------------------------------*/

NwIpv4RcT nwTimerStart( NwIpv4TimerMgrHandleT tmrMgrHandle,
    NwU32T timeoutSec,
    NwU32T timeoutUsec,
    NwU32T tmrType,
    void*  timeoutArg,
    NwIpv4TimerHandleT* hTmr)
{
  NwIpv4RcT rc = NW_IPv4_OK;

  NW_LOG(NW_LOG_LEVEL_INFO, "Received start timer request from stack with timer type %u, arg %x, for %u sec and %u usec", tmrType, timeoutArg, timeoutSec, timeoutUsec);

#ifdef __WITH_LIBEVENT__
  NwMiniTmrT *pTmr;
  struct timeval tv;
  pTmr = (NwMiniTmrT*) malloc (sizeof(NwMiniTmrT));

  /* set the timevalues*/
  timerclear(&tv);
  tv.tv_sec     = timeoutSec;
  tv.tv_usec    = timeoutUsec;

  pTmr->timeoutArg = timeoutArg;
  /*add event*/
  evtimer_set(&pTmr->ev, nwMiniTmrMgrHandleTimeout, pTmr);
  event_add(&(pTmr->ev), &tv);
  *hTmr = (NwIpv4TimerHandleT)pTmr;
#else
#warning "Timer library not defined!"
#endif


  return NW_IPv4_OK;
}

NwIpv4RcT nwTimerStop( NwIpv4TimerMgrHandleT tmrMgrHandle,
    NwIpv4TimerHandleT hTmr)
{
  NW_LOG(NW_LOG_LEVEL_INFO, "Received stop timer request from stack for timer handle %u", hTmr);
#ifdef __WITH_LIBEVENT__
  evtimer_del(&(((NwMiniTmrT*)hTmr)->ev));
  free((void*)hTmr);
#else
#warning "Timer library not defined!"
#endif
 
  return NW_IPv4_OK;
}

#ifdef __cplusplus
}
#endif

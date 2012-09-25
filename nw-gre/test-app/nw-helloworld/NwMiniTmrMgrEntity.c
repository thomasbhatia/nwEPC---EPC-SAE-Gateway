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
#include "NwGre.h"
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
  NwRcT rc;
#ifdef __WITH_LIBEVENT__
  NwMiniTmrMgrEntityT *pTmr = (NwMiniTmrMgrEntityT*) arg;

  /*---------------------------------------------------------------------------
   *  Send Timeout Request to GRE Stack Instance
   *--------------------------------------------------------------------------*/

  rc = nwGreProcessTimeout(pTmr->timeoutArg);
  NW_ASSERT( NW_OK == rc );

  free(pTmr);

#else
#warning "Timer library not defined!"
#endif

  return;
}

/*---------------------------------------------------------------------------
 * Public functions
 *--------------------------------------------------------------------------*/

NwRcT nwTimerStart( NwGreTimerMgrHandleT tmrMgrHandle,
    NwU32T timeoutSec,
    NwU32T timeoutUsec,
    NwU32T tmrType,
    void*  timeoutArg,
    NwGreTimerHandleT* hTmr)
{
  NwRcT rc = NW_OK;
#ifdef __WITH_LIBEVENT__
  NwMiniTmrMgrEntityT *pTmr;
  struct timeval tv;

  pTmr = (NwMiniTmrMgrEntityT*) malloc (sizeof(NwMiniTmrMgrEntityT));

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

  *hTmr = (NwGreTimerHandleT)pTmr;

  return NW_OK;
}

NwRcT nwTimerStop( NwGreTimerMgrHandleT tmrMgrHandle,
    NwGreTimerHandleT hTmr)
{
#ifdef __WITH_LIBEVENT__
  evtimer_del(&(((NwMiniTmrMgrEntityT*)hTmr)->ev));
  free((void*)hTmr);
#else
#warning "Timer library not defined!"
#endif
  return NW_OK;
}

#ifdef __cplusplus
}
#endif

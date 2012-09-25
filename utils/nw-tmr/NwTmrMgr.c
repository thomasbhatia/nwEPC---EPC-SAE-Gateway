/*----------------------------------------------------------------------------*
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
 * @file NwTmrMgr.c
 * @brief 
*/

#include <stdio.h>

#include "NwEvt.h"
#include "NwTypes.h"
#include "NwError.h"
#include "NwLog.h"
#include "NwMem.h"
#include "NwTmrMgr.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _EVENT_HAVE_TIMERADD
#define NW_EVENT_TIMER_ADD(tvp, uvp, vvp) timeradd((tvp), (uvp), (vvp))
#define NW_EVENT_TIMER_SUB(tvp, uvp, vvp) timersub((tvp), (uvp), (vvp))
#else

#define NW_EVENT_TIMER_ADD(tvp, uvp, vvp)                               \
    do {                                                                \
      (vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec;                    \
      (vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec;                 \
      if ((vvp)->tv_usec >= 1000000) {                                  \
        (vvp)->tv_sec++;                                                \
        (vvp)->tv_usec -= 1000000;                                      \
      }                                                                 \
    } while (0)

#define NW_EVENT_TIMER_SUB(tvp, uvp, vvp)                               \
    do {                                                                \
      (vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec;                    \
      (vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec;                 \
      if ((vvp)->tv_usec < 0) {                                         \
        (vvp)->tv_sec--;                                                \
        (vvp)->tv_usec += 1000000;                                      \
      }                                                                 \
    } while (0)
#endif

#ifdef __WITH_LIBEVENT__
typedef struct
{
  NwEventT ev;
  void   (*timeoutCallback)(int fd, short event, void *arg);
  void*  timeoutArg;
  NwU8T  tmrType;
  struct timeval timeout;
  struct timeval tv;
} NwTmrT;

static void
nwNwTmeMgrHandleTimerTimeout(int fd, short event, void *arg)
{
  NwTmrT *pTmr = (NwTmrT*) arg;
  pTmr->timeoutCallback(fd, event, pTmr->timeoutArg);
  if(pTmr->tmrType == NW_TIMER_TYPE_REPETITIVE)
  {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    NW_EVENT_TIMER_ADD(&pTmr->tv, &pTmr->timeout, &pTmr->tv);
    NW_EVENT_TIMER_SUB(&tv, &pTmr->tv, &tv);
    NW_EVENT_TIMER_SUB(&pTmr->timeout, &tv, &tv);

    /* Add event to event library */
    event_add(&(pTmr->ev), &tv);
  }
  else
  {
    evtimer_del(&(pTmr->ev));
    nwMemDelete((void*)pTmr);
  }
}
#endif

NwRcT 
nwTmrMgrInitialize( NwTimerMgrHandleT *tmrMgrHandle)
{
  NwRcT rc = NW_OK;
  return rc;
}

NwRcT 
nwTmrMgrFinalize( NwTimerMgrHandleT tmrMgrHandle)
{
  NwRcT rc = NW_OK;
  return rc;
}

NwRcT 
nwTmrMgrStartTimer( NwTimerMgrHandleT tmrMgrHandle,
    NwU32T timeoutSec,
    NwU32T timeoutUsec,
    NwU32T tmrType,
    void   NW_TMR_CALLBACK((*nwTimerTimeout)),
    void*  timeoutArg,
    NwTimerHandleT* phTmr)
{
  NwRcT rc = NW_OK;
#ifdef __WITH_LIBEVENT__
  NwTmrT *pTmr;

  pTmr = (NwTmrT*) nwMemNew (sizeof(NwTmrT));

  if(pTmr)
  {
    if(tmrType == NW_TIMER_TYPE_REPETITIVE)
    {
      gettimeofday(&pTmr->tv, NULL);
    }

    /* Set the timevalues*/

    pTmr->timeoutCallback = nwTimerTimeout;
    pTmr->timeoutArg      = timeoutArg;
    pTmr->tmrType         = tmrType;
    pTmr->timeout.tv_sec  = timeoutSec;
    pTmr->timeout.tv_usec = timeoutUsec;

    /* Add event to event library */

    evtimer_set(&pTmr->ev, nwNwTmeMgrHandleTimerTimeout, pTmr);
    event_add(&(pTmr->ev), &pTmr->timeout);
#else

    NwEventT* pTmr;
    rc = nwEventTimerCreateAndStart(&pTmr, nwTimerTimeout, timeoutArg, timeoutSec, timeoutUsec, (tmrType == NW_TIMER_TYPE_ONE_SHOT ? NW_EVENT_TIMER_ONE_SHOT : NW_EVENT_TIMER_REPETITIVE));

#endif

    *phTmr = (NwTimerHandleT)pTmr;
#ifdef __WITH_LIBEVENT__
  }
  else
  {
    rc = NW_FAILURE;
  }
#endif 

  return NW_OK;
}


NwRcT 
nwTmrMgrStopTimer( NwTimerMgrHandleT tmrMgrHandle,
    NwTimerHandleT hTmr)
{
#ifdef __WITH_LIBEVENT__
  evtimer_del(&(((NwTmrT*)hTmr)->ev));
  nwMemDelete((void*)hTmr);
#else
  nwEventTimerDestroy((NwEventT*)hTmr);
#endif
  return NW_OK;
}

#ifdef __cplusplus
}
#endif


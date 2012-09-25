/*----------------------------------------------------------------------------*
 *                                                                            *
 *                                n w - m m e                                 * 
 *    L T E / S A E    M O B I L I T Y   M A N A G E M E N T   E N T I T Y    *
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
 * @file nwMmeUlp.c
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "NwLog.h"
#include "NwUtils.h"
#include "NwMmeLog.h"
#include "NwLogMgr.h"
#include "NwGtpv2c.h"
#include "NwMmeUe.h"
#include "NwMmeUlp.h"
#include "NwUdp.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NW_MME_ULP_STATS_TIMEOUT                (10)
#define NW_MME_ULP_RPS_TIMEOUT                  (1)

static NwU32T statsTimerTicks = 1;
static void
NW_TMR_CALLBACK(nwMmeUlpHandleStatsTimerTimeout)
{
  NwRcT rc;
  NwMmeUlpT* thiz = arg;

  NW_MME_LOG(NW_LOG_LEVEL_CRIT, "Average registrations per sec : %f!", ((float)thiz->sessionCount)/(float)(NW_MME_ULP_STATS_TIMEOUT * statsTimerTicks));

  statsTimerTicks++;

  return;
}

static NwU32T rpsTimerTicks = 1;
static void
NW_TMR_CALLBACK(nwMmeUlpHandleRpsTimerTimeout)
{
  NwRcT rc;
  NwU32T rps;
  NwU32T currRps;
  NwMmeUeT *pMs;
  NwMmeUeEventInfoT eventInfo;
  NwMmeUlpT* thiz = arg;

  currRps = ((float)thiz->sessionCount)/(float)(NW_MME_ULP_RPS_TIMEOUT * rpsTimerTicks);
  rps = thiz->rps + (thiz->rps > currRps ? (thiz->rps - currRps) : 0 );

  pMs = thiz->pMsListDeregisteredHead;
  while(pMs && rps--)
  {
    NW_ASSERT(pMs->state == NW_MME_UE_STATE_INIT);
    eventInfo.event = NW_MME_UE_EVENT_NETWORK_ENTRY_START;
    rc = nwMmeUeFsmRun(pMs, &eventInfo);
    if(rc != NW_OK)
      break;
    pMs = pMs->deregisteredNext;
  }

  thiz->pMsListDeregisteredHead = pMs;
}

NwRcT
nwMmeUlpInit(NwMmeUlpT*         thiz, 
             NwU32T             maxUeSessions,
             NwU32T             sessionTimeout,
             NwU32T             mmeIpAddr,
             NwU32T             sgwIpAddr,
             NwU32T             pgwIpAddr,
             NwU32T             rps,
             NwMmeDpeT          *pDpe,
             NwGtpv2cStackHandleT hGtpcStack)
{
  NwRcT rc;

  memset(thiz, 0, sizeof(NwMmeUlpT));

  thiz->rps             = rps;
  thiz->maxUeSessions   = maxUeSessions;
  thiz->hGtpcStack      = hGtpcStack;
  thiz->mmeIpAddr       = mmeIpAddr;
  thiz->sgwIpAddr       = sgwIpAddr;
  thiz->pgwIpAddr       = pgwIpAddr;
  thiz->sessionTimeout  = sessionTimeout;
  thiz->pDpe            = pDpe;

  return NW_OK;
}

NwRcT
nwMmeUlpDestroy(NwMmeUlpT*  thiz) 
{
  NwRcT rc;
  memset(thiz, 0, sizeof(NwMmeUlpT));
  return NW_OK;
}

NwRcT
nwMmeUlpCreateConn(NwMmeUlpT* thiz)
{
  NwRcT rc;
  NwU32T i;
  NwGtpv2cUlpApiT  ulpReq;
  NwMmeUeT* pMs;

  for(i = 0; i<thiz->maxUeSessions; i++)
  {
    pMs = nwMmeUeNew(thiz->hGtpcStack);

    pMs->hMmeUlp        = (NwU32T) thiz; 
    pMs->pDpe           = thiz->pDpe; 
    pMs->mmeIpv4Addr    = thiz->mmeIpAddr; 
    pMs->sgwIpv4Addr    = thiz->sgwIpAddr; 
    pMs->pgwIpv4Addr    = thiz->pgwIpAddr; 
    pMs->sessionTimeout = thiz->sessionTimeout;

    if(thiz->pMsListHead == NULL)
    {
      thiz->pMsListHead = pMs;
      thiz->pMsListTail = pMs;
    }
    else
    {
      thiz->pMsListTail->next = pMs;
      thiz->pMsListTail = pMs;
    }

    if(thiz->pMsListDeregisteredHead == NULL)
    {
      thiz->pMsListDeregisteredHead = pMs;
      thiz->pMsListDeregisteredTail = pMs;
      pMs->deregisteredNext = NULL;
    }
    else
    {
      thiz->pMsListDeregisteredTail->deregisteredNext = pMs;
      thiz->pMsListDeregisteredTail = pMs;
      pMs->deregisteredNext = NULL;
    }

  /*---------------------------------------------------------------------------
   *  Send Create session Request to GTPv2c Stack Instance
   *--------------------------------------------------------------------------*/

  }

  return NW_OK;
}

NwRcT
nwMmeUlpDestroyConn(NwMmeUlpT* thiz)
{
  NwRcT rc;
  NwGtpv2cUlpApiT           ulpReq;
  /*---------------------------------------------------------------------------
   *  Send Destroy session Request to GTPv2c Stack Instance
   *--------------------------------------------------------------------------*/

  return NW_OK;
}

NwRcT
nwMmeUlpGetControlPlaneIpv4Addr(NwMmeUlpT* thiz, NwU32T *pMmeControlPlaneIpv4Addr)
{
  if(thiz) 
  {
    *pMmeControlPlaneIpv4Addr = thiz->mmeIpAddr;
    return NW_OK;
  }
  return NW_FAILURE;
}

NwRcT
nwMmeUlpStartNetworkEntry(NwMmeUlpT* thiz)
{
  NwRcT rc;
  NwGtpv2cUlpApiT               ulpReq;
  NwMmeUeT*                     pMs;
  NwMmeUeEventInfoT             eventInfo;

  rc = nwMmeUlpCreateConn(thiz);
  NW_ASSERT( NW_OK == rc );

  rc = nwTmrMgrStartTimer(0, NW_MME_ULP_RPS_TIMEOUT, 0, NW_TIMER_TYPE_REPETITIVE, nwMmeUlpHandleRpsTimerTimeout, thiz, &thiz->hStatsTimer); 
  NW_ASSERT(NW_OK == rc);

  rc = nwTmrMgrStartTimer(0, NW_MME_ULP_STATS_TIMEOUT, 0, NW_TIMER_TYPE_REPETITIVE, nwMmeUlpHandleStatsTimerTimeout, thiz, &thiz->hStatsTimer); 
  NW_ASSERT(NW_OK == rc);

  return rc ;
}


NwRcT 
nwMmeUlpStackReqCallback (NwGtpv2cUlpHandleT hUlp, 
                       NwGtpv2cUlpApiT *pUlpApi)
{
  NwRcT rc;
  NwMmeUlpT* thiz;
  NwMmeUeEventInfoT             eventInfo;

  NW_ASSERT(pUlpApi != NULL);

  thiz = (NwMmeUlpT*) hUlp;

  switch(pUlpApi->apiType)
  {
    case NW_GTPV2C_ULP_API_INITIAL_REQ_IND:
      {
        eventInfo.event = NW_MME_UE_EVENT_GTPC_MSG_INDICATION;
        eventInfo.arg   = pUlpApi;
        NW_MME_LOG(NW_LOG_LEVEL_DEBG, "Received NW_GTPV2C_ULP_API_INITIAL_REQ_IND from GTPv2c stack!");
        rc = nwMmeUeFsmRun((NwMmeUeT*)pUlpApi->apiInfo.initialReqIndInfo.hUlpTrxn, &eventInfo);
      }
      break;

    case NW_GTPV2C_ULP_API_TRIGGERED_RSP_IND:
      {
        eventInfo.event = NW_MME_UE_EVENT_GTPC_TRIGGERED_RSP_MSG_INDICATION;
        eventInfo.arg   = pUlpApi;
        NW_MME_LOG(NW_LOG_LEVEL_DEBG, "Received NW_GTPV2C_ULP_API_TRIGGERED_RSP_IND from GTPv2c stack!");
        rc = nwMmeUeFsmRun((NwMmeUeT*)pUlpApi->apiInfo.triggeredRspIndInfo.hUlpTrxn, &eventInfo);
      }
      break;

    case NW_GTPV2C_ULP_API_TRIGGERED_REQ_IND:
      {
        eventInfo.event = NW_MME_UE_EVENT_GTPC_MSG_INDICATION;
        eventInfo.arg   = pUlpApi;
        NW_MME_LOG(NW_LOG_LEVEL_DEBG, "Received NW_GTPV2C_ULP_API_TRIGGERED_REQ_IND from GTPv2c stack!");
        rc = nwMmeUeFsmRun((NwMmeUeT*)pUlpApi->apiInfo.triggeredReqIndInfo.hUlpTrxn, &eventInfo);
      }
      break;

    case NW_GTPV2C_ULP_API_RSP_FAILURE_IND:
      {
        eventInfo.event   = NW_MME_UE_EVENT_NACK;
        eventInfo.arg     = pUlpApi;
        NW_MME_LOG(NW_LOG_LEVEL_DEBG, "Received NW_GTPV2C_ULP_API_RSP_FAILURE from GTPv2c stack for session 0x%x and transaction 0x%x!",
            pUlpApi->apiInfo.rspFailureInfo.hUlpTrxn);

        rc = nwMmeUeFsmRun((NwMmeUeT*)pUlpApi->apiInfo.rspFailureInfo.hUlpTrxn, &eventInfo);
      }

      break;

    default:
      NW_MME_LOG(NW_LOG_LEVEL_WARN, "Received undefined api type %x from GTPv2c stack!", pUlpApi->apiType);
  }

  if(pUlpApi->hMsg)
    rc = nwGtpv2cMsgDelete(thiz->hGtpcStack, (pUlpApi->hMsg));

  return rc;
}


#ifdef __cplusplus
}
#endif


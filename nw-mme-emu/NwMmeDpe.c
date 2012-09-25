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
 * @file NwMmeDpe.c
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "NwEvt.h"
#include "NwLog.h"
#include "NwMem.h"
#include "NwTypes.h"
#include "NwUtils.h"
#include "NwTmrMgr.h"
#include "NwLogMgr.h"
#include "NwMmeDpe.h"
#include "NwMmeLog.h"
#include "NwIpv4If.h"

#ifdef __cplusplus
extern "C" {
#endif


static void
NW_EVT_CALLBACK(nwMmeDpeHandleSdpTimerTimeout)
{
  NwRcT rc;
  /*
   *  Send Timeout Request to Soft Data Plane Instance
   */
  rc = nwSdpProcessTimeout(arg);
  NW_ASSERT( NW_OK == rc );

  return;
}

static
NwRcT nwMmeDpeSdpTimerStartIndication( NwSdpTimerMgrHandleT tmrMgrHandle,
    NwU32T timeoutSec,
    NwU32T timeoutUsec,
    NwU32T tmrType,
    void*  timeoutArg,
    NwSdpTimerHandleT* phTmr)
{
  return nwTmrMgrStartTimer(tmrMgrHandle, timeoutSec, timeoutUsec, tmrType, nwMmeDpeHandleSdpTimerTimeout, timeoutArg, (NwTimerHandleT*)phTmr);
}

static
NwRcT nwMmeDpeSdpTimerStopIndication( NwSdpTimerMgrHandleT tmrMgrHandle,
    NwSdpTimerHandleT hTmr)
{
  return nwTmrMgrStopTimer(tmrMgrHandle, (NwTimerHandleT)hTmr);
}

static
void* nwMmeDpeSdpMemNewIndication(NwSdpMemMgrHandleT hMemMgr, NwU32T size, NwCharT* fn, NwU32T ln)
{
  return _nwMemNew(size, fn, ln);
}

static
void  nwMmeDpeSdpMemFreeIndication(NwSdpMemMgrHandleT hMemMgr, void* mem, NwCharT* fn, NwU32T ln)
{
  return _nwMemDelete(mem, fn, ln);
}

static
NwRcT nwMmeDpeHandleSdpLogRequest(NwSdpLogMgrHandleT hlogMgr,
    NwU32T logLevel,
    NwCharT* file,
    NwU32T line,
    NwCharT* logStr)
{
  NwLogMgrT* thiz = (NwLogMgrT*) hlogMgr;
  if(thiz->logLevel >= logLevel)
  {
    nwLogMgrLog(&_gLogMgr, " NW-DPE  ", logLevel, (char*)file, line, logStr);
  }
  return NW_OK;
}

NwMmeDpeT*
nwMmeDpeInitialize()
{
  NwRcT rc;
  NwSdpMemMgrEntityT            memMgr;
  NwSdpTimerMgrEntityT          tmrMgr;
  NwSdpLogMgrEntityT            logMgr;
  NwMmeDpeT* thiz;

  thiz = (NwMmeDpeT*) nwMemNew (sizeof(NwMmeDpeT));

  memset(thiz, 0, sizeof(NwMmeDpeT));

  /*---------------------------------------------------------------------------
   * Create Soft Data Plane Instance
   *--------------------------------------------------------------------------*/
  rc = nwSdpInitialize(&thiz->hSdp);

  if(rc != NW_OK)
  {
    NW_MME_LOG(NW_LOG_LEVEL_ERRO, "Failed to create Soft Data Plane instance. Error '%u' occured", rc);
    exit(1);
  }

  NW_MME_LOG(NW_LOG_LEVEL_INFO, "SDP Handle '%X' Creation Successful!", thiz->hSdp);

  /*
   * Set up Memory Manager Entity 
   */

  memMgr.hMemMgr                = 0;
  memMgr.memAlloc               = nwMmeDpeSdpMemNewIndication;
  memMgr.memFree                = nwMmeDpeSdpMemFreeIndication;

  rc = nwSdpSetMemMgrEntity(thiz->hSdp, &memMgr);
  NW_ASSERT( NW_OK == rc );


  /* 
   * Set up Log Entity 
   */ 

  logMgr.logMgrHandle           = (NwSdpLogMgrHandleT) nwLogMgrGetInstance();
  logMgr.logReqCallback         = nwMmeDpeHandleSdpLogRequest;

  rc = nwSdpSetLogMgrEntity(thiz->hSdp, &logMgr);
  NW_ASSERT( NW_OK == rc );

  rc = nwSdpSetLogLevel(thiz->hSdp, nwLogMgrGetLogLevel(nwLogMgrGetInstance()));
  NW_ASSERT( NW_OK == rc );

  /*
   * Set up Timer Manager Entity 
   */

  tmrMgr.tmrMgrHandle           = 0;
  tmrMgr.tmrStartCallback       = nwMmeDpeSdpTimerStartIndication;
  tmrMgr.tmrStopCallback        = nwMmeDpeSdpTimerStopIndication;

  rc = nwSdpSetTimerMgrEntity(thiz->hSdp, &tmrMgr);
  NW_ASSERT( NW_OK == rc );

  return thiz;
}

NwRcT
nwMmeDpeCreateGtpuService( NwMmeDpeT* thiz, NwU32T ipv4Addr )
{
  NwRcT rc;
  NwU32T gtpuSelObj;

  /* Initialize and Set up Gtpv1u Udp Entity */

  rc = nwGtpv1uIfInitialize(&thiz->gtpuIf, ipv4Addr, thiz->hSdp);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv1uIfGetSelectionObject(&thiz->gtpuIf, &gtpuSelObj);
  NW_ASSERT( NW_OK == rc );

  NW_EVENT_ADD((thiz->evGtpuIf), gtpuSelObj, nwGtpv1uIfDataIndicationCallback, &thiz->gtpuIf, (NW_EVT_READ | NW_EVT_PERSIST));

  rc = nwSdpCreateGtpuService(thiz->hSdp,
                              (NwU32T)&thiz->gtpuIf,
                              nwGtpv1uIfDataReq,
                              (NwSdpServiceHandleT*)&thiz->hGtpu);
  NW_ASSERT( NW_OK == rc );
  return rc;
}

NwRcT
nwMmeDpeCreateIpv4Service( NwMmeDpeT* thiz, NwU8T* nwIfName)
{
  NwRcT rc;
  NwU32T selObj;
  NwU8T hwAddr[6];

  /* Initialize and Set up IPv4 Entity */

  rc = nwIpv4IfInitialize(&thiz->ipv4If, nwIfName, thiz->hSdp, hwAddr);
  NW_ASSERT( NW_OK == rc );

  rc = nwSdpCreateIpv4Service(thiz->hSdp,
                              NW_SDP_IPv4_MODE_UPLINK,
                              hwAddr,
                              (NwU32T)&thiz->ipv4If,
                              nwIpv4IfIpv4DataReq,
                              nwIpv4IfArpDataReq,
                              &thiz->hIpv4);
  rc = nwIpv4IfGetSelectionObjectIpv4(&thiz->ipv4If, &selObj);
  NW_ASSERT( NW_OK == rc );

  NW_EVENT_ADD((thiz->evIpv4), selObj, nwIpv4IfDataIndicationCallback, &thiz->ipv4If, (NW_EVT_READ | NW_EVT_PERSIST));

  NW_ASSERT( NW_OK == rc );
  return rc;
}

NwRcT
nwMmeDpeDestroy(NwMmeDpeT* thiz)
{
  /* TODO */
  NW_ASSERT(0);
}

/**
 * Create Gtpu to Ipv4 flow with Soft Data Plane
 */

NwRcT
nwMmeDpeCreateGtpuIpv4Flow(NwMmeDpeT*   thiz, 
                         NwU32T         hSession,
                         NwU32T         teidIngress,
                         NwU32T         *pTeidIngress,
                         NwU32T         *pIpv4Ingress,
                         NwU32T         *phBearer)
{
  NwRcT rc;
  NwSdpUlpApiT           ulpReq;

  ulpReq.apiType                              = NW_SDP_ULP_API_CREATE_FLOW;
  ulpReq.apiInfo.createFlowInfo.hUlpSession   = (NwSdpUlpSessionHandleT) hSession;

  ulpReq.apiInfo.createFlowInfo.ingressEndPoint.ipv4Addr                = thiz->gtpuIf.ipAddr;
  ulpReq.apiInfo.createFlowInfo.ingressEndPoint.flowType                = NW_FLOW_TYPE_GTPU;
  ulpReq.apiInfo.createFlowInfo.ingressEndPoint.flowKey.gtpuTeid        = teidIngress;

  ulpReq.apiInfo.createFlowInfo.egressEndPoint.flowType                 = NW_FLOW_TYPE_IPv4;

  rc = nwSdpProcessUlpReq(thiz->hSdp, &ulpReq);
  NW_ASSERT( NW_OK == rc );

  *pTeidIngress  = ulpReq.apiInfo.createFlowInfo.ingressEndPoint.flowKey.gtpuTeid; 
  *pIpv4Ingress  = ulpReq.apiInfo.createFlowInfo.ingressEndPoint.ipv4Addr;
  *phBearer     = (NwU32T) ulpReq.apiInfo.createFlowInfo.hSdpSession; 
  return rc;
}

/**
 * Create Ipv4 to Gtpu flow with Soft Data Plane
 */

NwRcT
nwMmeDpeCreateIpv4GtpuFlow(NwMmeDpeT*   thiz, 
                         NwU32T         hSession,
                         NwU32T         teidEgress,
                         NwU32T         ipv4Egress,
                         NwU32T         ipv4Ingress,
                         NwU32T         *phBearer)
{
  NwRcT rc;
  NwSdpUlpApiT           ulpReq;

  ulpReq.apiType                              = NW_SDP_ULP_API_CREATE_FLOW;
  ulpReq.apiInfo.createFlowInfo.hUlpSession   = (NwSdpUlpSessionHandleT) hSession;

  ulpReq.apiInfo.createFlowInfo.ingressEndPoint.flowType                = NW_FLOW_TYPE_IPv4;
  ulpReq.apiInfo.createFlowInfo.ingressEndPoint.flowKey.ipv4Addr        = ipv4Ingress;

  ulpReq.apiInfo.createFlowInfo.egressEndPoint.ipv4Addr                 = ipv4Egress;
  ulpReq.apiInfo.createFlowInfo.egressEndPoint.flowType                 = NW_FLOW_TYPE_GTPU;
  ulpReq.apiInfo.createFlowInfo.egressEndPoint.flowKey.gtpuTeid         = teidEgress;

  rc = nwSdpProcessUlpReq(thiz->hSdp, &ulpReq);
  NW_ASSERT( NW_OK == rc );

  *phBearer = (NwU32T) ulpReq.apiInfo.createFlowInfo.hSdpSession; 
  return rc;
}

/**
 * Create Gtpu to Gtpu flow with Soft Data Plane
 */

NwRcT
nwMmeDpeCreateGtpuGtpuFlow(NwMmeDpeT*   thiz, 
                         NwU32T         hSession,
                         NwU32T         teidEgress,
                         NwU32T         ipv4Egress,
                         NwU32T         *teidIngress,
                         NwU32T         *ipv4Ingress,
                         NwU32T         *phBearer)
{
  NwRcT                 rc;
  NwSdpUlpApiT          ulpReq;

  ulpReq.apiType                              = NW_SDP_ULP_API_CREATE_FLOW;
  ulpReq.apiInfo.createFlowInfo.hUlpSession   = (NwSdpUlpSessionHandleT) hSession;

  ulpReq.apiInfo.createFlowInfo.ingressEndPoint.ipv4Addr                = thiz->gtpuIf.ipAddr;
  ulpReq.apiInfo.createFlowInfo.ingressEndPoint.flowType                = NW_FLOW_TYPE_GTPU;
  ulpReq.apiInfo.createFlowInfo.ingressEndPoint.flowKey.gtpuTeid        = 1234;

  ulpReq.apiInfo.createFlowInfo.egressEndPoint.ipv4Addr                 = ipv4Egress;
  ulpReq.apiInfo.createFlowInfo.egressEndPoint.flowType                 = NW_FLOW_TYPE_GTPU;
  ulpReq.apiInfo.createFlowInfo.egressEndPoint.flowKey.gtpuTeid         = teidEgress;

  rc = nwSdpProcessUlpReq(thiz->hSdp, &ulpReq);
  NW_ASSERT( NW_OK == rc );

  *teidIngress  = ulpReq.apiInfo.createFlowInfo.ingressEndPoint.flowKey.gtpuTeid;
  *ipv4Ingress  = ulpReq.apiInfo.createFlowInfo.ingressEndPoint.ipv4Addr;

  *phBearer       = (NwU32T) ulpReq.apiInfo.createFlowInfo.hSdpSession; 

  return rc;
}

/**
 * Destroy a flow with Soft Data Plane
 */

NwRcT
nwMmeDpeDestroyFlow(NwMmeDpeT*   thiz, 
                    NwU32T         hBearer)
{
  NwRcT rc;
  NwSdpUlpApiT           ulpReq;

  ulpReq.apiType                              = NW_SDP_ULP_API_DESTROY_FLOW;
  ulpReq.apiInfo.destroyFlowInfo.hSdpSession  = (NwSdpSessionHandleT) hBearer;

  rc = nwSdpProcessUlpReq(thiz->hSdp, &ulpReq);
  NW_ASSERT( NW_OK == rc );

  return rc;
}

#ifdef __cplusplus
}
#endif

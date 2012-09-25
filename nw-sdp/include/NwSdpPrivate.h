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

#ifndef __NW_SDP_PRIVATE_H__
#define __NW_SDP_PRIVATE_H__

#include "tree.h"
#include "queue.h"
#include "NwTypes.h"
#include "NwSdpError.h"
#include "NwSdp.h"

#include "NwGtpv1u.h"
#include "NwGre.h"
#include "NwIpv4.h"


/** 
 * @file NwSdpPrivate.h
 * @brief This header file contains private definitions not to be 
 * exposed to user application.
 */

#ifdef __cplusplus
extern "C" {
#endif

#define NW_SDP_MALLOC(_sdp, _size, _mem, _type)                         \
  do {                                                                  \
    if(((NwSdpT*)(_sdp))->memMgr.memAlloc && ((NwSdpT*)(_sdp))->memMgr.memFree )\
    {                                                                   \
      _mem = (_type) ((NwSdpT*) (_sdp))->memMgr.memAlloc(((NwSdpT*) (_sdp))->memMgr.hMemMgr, _size, __FILE__, __LINE__);\
    }                                                                   \
    else                                                                \
    {                                                                   \
      _mem = (_type) malloc (_size);                                    \
    }                                                                   \
  } while (0)

#define NW_SDP_FREE(_sdp, _mem)                                         \
  do {                                                                  \
    if(((NwSdpT*)(_sdp))->memMgr.memAlloc && ((NwSdpT*)(_sdp))->memMgr.memFree )\
    {                                                                   \
      ((NwSdpT*)(_sdp))->memMgr.memFree(((NwSdpT*) (_sdp))->memMgr.hMemMgr, _mem, __FILE__, __LINE__);\
    }                                                                   \
    else                                                                \
    {                                                                   \
      free ((void*)_mem);                                               \
    }                                                                   \
  } while (0)


/*--------------------------------------------------------------------------*
 *           S D P    O B J E C T   T Y P E    D E F I N I T I O N          *
 *--------------------------------------------------------------------------*/

/**
 * SDP class definition
 */

typedef struct NwSdp
{
  NwU32T                        id;
  NwU32T                        seq;
  NwU32T                        logLevel;
  NwSdpUlpEntityT               ulp;
  NwSdpUdpEntityT               udp;
  NwSdpMemMgrEntityT            memMgr;
  NwSdpTimerMgrEntityT          tmrMgr;
  NwSdpLogMgrEntityT            logMgr;
  NwGtpv1uStackHandleT          hGtpv1uStack;
  NwGreStackHandleT             hGreStack;
  NwIpv4StackHandleT            hIpv4Stack;
  NwU32T                        greSd;
} NwSdpT; 

/**
 * GTP Tunnel End Point class definition
 */

typedef struct NwSdpFlowContext
{
  NwSdpFlowEndPointT            ingressEndPoint;
  NwSdpFlowEndPointT            egressEndPoint;
  NwU32T                        egressIpv4Addr;                 /**< Egress Ip Address for this session */
  NwSdpT*                       pStack;                         /**< Pointer to the parent stack        */
  NwSdpUlpSessionHandleT        hUlpSession;                    /**< ULP session handle for the session */
  struct NwSdpFlowContext       *next;
} NwSdpFlowContextT;


/*--------------------------------------------------------------------------*
 * Timeout Info Type Definition  
 *--------------------------------------------------------------------------*/

/**
 * Timeout info 
 */

typedef struct NwSdpTimeoutInfo
{
  NwSdpHandleT hStack;
  void* timeoutArg;
  NwSdpRcT (*timeoutCallbackFunc)(void*);
} NwSdpTimeoutInfoT;

/**
 * Start a transaction response timer
 *
 * @param[in] thiz Pointer to stack instance
 * @param[in] timeoutArg Arg to timeout function.
 * @param[out] phTmr Pointer to timer handle. 
 * @return NW_SDP_OK on success.
 */

NwSdpRcT
nwSdpStartTrxnPeerRspTimer(NwSdpT* thiz, NwSdpTimeoutInfoT* timeoutInfo, NwSdpTimerHandleT* phTmr);

/**
 * Stop a transaction response timer
 *
 * @param[in] thiz Pointer to stack instance
 * @param[out] phTmr Pointer to timer handle. 
 * @return NW_SDP_OK on success.
 */

NwSdpRcT
nwSdpStopTrxnPeerRspTimer(NwSdpT* thiz, NwSdpTimerHandleT* phTmr);


#define NW_SDP_MAX_MSG_LEN                                    (4096)  /**< Maximum supported gre packet length including header */

/**
 * NwSdpMsgT holds gre messages to/from the peer.
 */
typedef struct NwSdpMsg
{
  NwU8T         version;
  NwU16T        protocolType;
  NwU8T         csumPresent;
  NwU8T         keyPresent;
  NwU16T        seqNumPresent;
  NwU32T        msgType;
  NwU16T        msgLen;
  NwU32T        teid;
  NwU16T        seqNum;
  NwU8T         npduNum;    
  NwU8T         nextExtHdrType;
  NwU8T         msgBuf[NW_SDP_MAX_MSG_LEN];
} NwSdpMsgT;


/**
 * GRE message header structure
 */

#pragma pack(1)

typedef struct NwSdpMsgHeader 
{
  NwU8T PN:1;
  NwU8T S:1;
  NwU8T E:1;
  NwU8T spare:1;
  NwU8T PT:1;
  NwU8T version:3;
  NwU8T msgType;
  NwU16T msgLength;
  NwU32T teid;
} NwSdpMsgHeaderT;

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif  /* __NW_SDP_PRIVATE_H__ */
/*--------------------------------------------------------------------------*
 *                      E N D     O F    F I L E                            *
 *--------------------------------------------------------------------------*/


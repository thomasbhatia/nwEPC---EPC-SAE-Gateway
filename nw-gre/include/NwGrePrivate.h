/*----------------------------------------------------------------------------*
 *                                                                            *
 *                             n w - g t p v 2 u                              * 
 *  G e n e r i c    R o u t i n g    E n c a p s u l a t i o n    S t a c k  *
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

#ifndef __NW_GRE_PRIVATE_H__
#define __NW_GRE_PRIVATE_H__

#include "tree.h"
#include "queue.h"

#include "NwTypes.h"
#include "NwGreError.h"
#include "NwGre.h"
#include "NwGreMsg.h"


/** 
 * @file NwGrePrivate.h
 * @brief This header file contains nw-gre private definitions not to be 
 * exposed to user application.
*/

#ifdef __cplusplus
extern "C" {
#endif

#define NW_GRE_MALLOC(_stack, _size, _mem, _type)                       \
  do {                                                                  \
    if(((NwGreStackT*)(_stack))->memMgr.memAlloc && ((NwGreStackT*)(_stack))->memMgr.memFree )\
    {                                                                   \
      _mem = (_type) ((NwGreStackT*) (_stack))->memMgr.memAlloc(((NwGreStackT*) (_stack))->memMgr.hMemMgr, _size, __FILE__, __LINE__);\
    }                                                                   \
    else                                                                \
    {                                                                   \
      _mem = (_type) malloc (_size);                                    \
    }                                                                   \
  } while (0)

#define NW_GRE_FREE(_stack, _mem)                                       \
  do {                                                                  \
    if(((NwGreStackT*)(_stack))->memMgr.memAlloc && ((NwGreStackT*)(_stack))->memMgr.memFree )\
    {                                                                   \
      ((NwGreStackT*)(_stack))->memMgr.memFree(((NwGreStackT*) (_stack))->memMgr.hMemMgr, _mem, __FILE__, __LINE__);\
    }                                                                   \
    else                                                                \
    {                                                                   \
      free ((void*)_mem);                                               \
    }                                                                   \
  } while (0)


/*--------------------------------------------------------------------------*
 *     G R E   S T A C K   O B J E C T   T Y P E    D E F I N I T I O N     *
 *--------------------------------------------------------------------------*/

/**
 * GRE stack class definition
 */

typedef struct NwGreStack
{
  NwU32T                        id;
  NwU32T                        seq;
  NwU32T                        logLevel;
  NwGreUlpEntityT               ulp;
  NwGreLlpEntityT               udp;
  NwGreMemMgrEntityT            memMgr;
  NwGreTimerMgrEntityT          tmrMgr;
  NwGreLogMgrEntityT            logMgr;
  RB_HEAD( NwGreOutstandingTxSeqNumTrxnMap, NwGreTrxn) outstandingTxSeqNumMap;
  RB_HEAD( NwGreOutstandingRxSeqNumTrxnMap, NwGreTrxn) outstandingRxSeqNumMap;
  RB_HEAD(NwGreTunnelEndPointTMap, NwGreTunnelEndPoint) sessionMap;
  RB_HEAD(NwGreTunnelEndPointIdentifierMap, NwGreTunnelEndPoint) teidMap;
} NwGreStackT; 

/**
 * GTP Tunnel End Point class definition
 */

typedef struct NwGreTunnelEndPoint
{
  struct NwGreTunnelEndPoint    *next;
  NwU32T                        greKey;                                 /**< Gre Tunnel End Point Identifier    */
  NwU32T                        peerAddr;                               /**< Peer IP address for the session    */
  NwGreStackT*                  pStack;                                 /**< Pointer to the parent stack        */
  NwGreUlpSessionHandleT        hUlpSession;                            /**< ULP session handle for the session */
  RB_ENTRY (NwGreTunnelEndPoint)    sessionMapRbtNode;                  /**< RB Tree Data Structure Node        */
} NwGreTunnelEndPointT;


/*--------------------------------------------------------------------------*
 * Timeout Info Type Definition  
 *--------------------------------------------------------------------------*/

/**
 * gre timeout info 
 */

typedef struct NwGreTimeoutInfo
{
  NwGreStackHandleT hStack;
  void* timeoutArg;
  NwRcT (*timeoutCallbackFunc)(void*);
} NwGreTimeoutInfoT;

/**
 * Start a transaction response timer
 *
 * @param[in] thiz Pointer to stack instance
 * @param[in] timeoutArg Arg to timeout function.
 * @param[out] phTmr Pointer to timer handle. 
 * @return NW_OK on success.
 */

NwRcT
nwGreStartTrxnPeerRspTimer(NwGreStackT* thiz, NwGreTimeoutInfoT* timeoutInfo, NwGreTimerHandleT* phTmr);

/**
 * Stop a transaction response timer
 *
 * @param[in] thiz Pointer to stack instance
 * @param[out] phTmr Pointer to timer handle. 
 * @return NW_OK on success.
 */

NwRcT
nwGreStopTrxnPeerRspTimer(NwGreStackT* thiz, NwGreTimerHandleT* phTmr);


#define NW_GRE_MAX_MSG_LEN                                    (4096)  /**< Maximum supported gre packet length including header */

/**
 * NwGreMsgT holds gre messages to/from the peer.
 */
typedef struct NwGreMsg
{
  struct NwGreMsg*              next;
  NwU8T                         version;
  NwU16T                        protocolType;
  NwU8T                         csumPresent;
  NwU8T                         keyPresent;
  NwU16T                        seqNumPresent;
  NwU32T                        msgType;
  NwU16T                        msgLen;
  NwU32T                        greKey;
  NwU16T                        seqNum;
  NwU8T                         npduNum;    
  NwU8T                         nextExtHdrType;
  NwU8T                         msgBuf[NW_GRE_MAX_MSG_LEN];
} NwGreMsgT;


/**
 * Transaction structure
 */

typedef struct NwGreTrxn
{
  struct NwGreTrxn*             next;
  NwU32T                        seqNum;
  NwU32T                        peerIp;
  NwU32T                        peerPort;
  NwU8T                         maxRetries;
  NwU8T                         stateIndex;
  NwU8T                         t3Timer;
  NwGreTimerHandleT             hRspTmr;
  NwGreTimeoutInfoT             peerRspTimeoutInfo;
  NwGreStackT*                  pStack;
  NwGreTunnelEndPointT*         pSession;
  NwU32T                        hUlpTrxn;
  NwGreMsgT*                    pMsg;
  RB_ENTRY (NwGreTrxn)          outstandingTxSeqNumMapRbtNode;            /**< RB Tree Data Structure Node        */
  RB_ENTRY (NwGreTrxn)          outstandingRxSeqNumMapRbtNode;            /**< RB Tree Data Structure Node        */
} NwGreTrxnT;


/**
 * GRE message header structure
 */

#pragma pack(1)

typedef struct NwGreMsgHeader 
{
  NwU8T PN:1;
  NwU8T S:1;
  NwU8T E:1;
  NwU8T spare:1;
  NwU8T PT:1;
  NwU8T version:3;
  NwU8T msgType;
  NwU16T msgLength;
  NwU32T greKey;
} NwGreMsgHeaderT;

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif  /* __NW_GRE_PRIVATE_H__ */
/*--------------------------------------------------------------------------*
 *                      E N D     O F    F I L E                            *
 *--------------------------------------------------------------------------*/


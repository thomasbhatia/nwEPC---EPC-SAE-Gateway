/*----------------------------------------------------------------------------*
 *                                                                            *
 *                               n w - i p v 4                                * 
 *           I n t e r n e t    P r o t o c o l    v 4    S t a c k           *
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

#ifndef __NW_IPv4_PRIVATE_H__
#define __NW_IPv4_PRIVATE_H__

#include "tree.h"
#include "queue.h"

#include "NwTypes.h"
#include "NwIpv4Error.h"
#include "NwIpv4.h"
#include "NwIpv4Msg.h"


/** 
 * @file NwIpv4Private.h
 * @brief This header file contains nw-Ipv4 private definitions not to be 
 * exposed to user application.
*/

#ifdef __cplusplus
extern "C" {
#endif

#define NW_IPv4_MALLOC(_stack, _size, _mem, _type)                      \
  do {                                                                  \
    _mem = (_type) malloc (_size);                                    \
    break;                                                              \
    if(((NwIpv4StackT*)(_stack))->memMgr.memAlloc && ((NwIpv4StackT*)(_stack))->memMgr.memFree )\
    {                                                                   \
      _mem = (_type) ((NwIpv4StackT*) (_stack))->memMgr.memAlloc(((NwIpv4StackT*) (_stack))->memMgr.hMemMgr, _size, __FILE__, __LINE__);\
    }                                                                   \
    else                                                                \
    {                                                                   \
      _mem = (_type) malloc (_size);                                    \
    }                                                                   \
  } while (0)



/*--------------------------------------------------------------------------*
 *    I P v 4   S T A C K   O B J E C T   T Y P E    D E F I N I T I O N    *
 *--------------------------------------------------------------------------*/

/**
 * IPv4 stack class definition
 */

typedef struct NwIpv4Stack
{
  NwU32T                        id;
  NwU32T                        mode;           /* Uplink or Downlink. Default is downlink. */
  NwU32T                        seq;
  NwU32T                        logLevel;
  NwIpv4UlpEntityT              ulp;
  NwIpv4LlpEntityT              llp;
  NwIpv4MemMgrEntityT           memMgr;
  NwIpv4TimerMgrEntityT         tmrMgr;
  NwIpv4LogMgrEntityT           logMgr;
  RB_HEAD(NwIpv4TunnelEndPointIdentifierMap, NwIpv4TunnelEndPoint) ipv4AddrMap;
} NwIpv4StackT; 

/**
 * Tunnel End Point class definition
 */

typedef struct NwIpv4TunnelEndPoint
{
  struct NwIpv4TunnelEndPoint   *next;
  NwU32T                        ipv4Addr;               /**< Tunnel End Point Identifier   */
  NwIpv4StackT*                 pStack;                 /**< Pointer to the parent stack        */
  NwIpv4UlpSessionHandleT       hUlpSession;            /**< ULP session handle for the session */
  RB_ENTRY (NwIpv4TunnelEndPoint)    sessionMapRbtNode; /**< RB Tree Data Structure Node        */
} NwIpv4TunnelEndPointT;


/*--------------------------------------------------------------------------*
 * Timeout Info Type Definition  
 *--------------------------------------------------------------------------*/

/**
 * timeout info 
 */

typedef struct NwIpv4TimeoutInfo
{
  NwIpv4StackHandleT hStack;
  void* timeoutArg;
  NwIpv4RcT (*timeoutCallbackFunc)(void*);
} NwIpv4TimeoutInfoT;

/**
 * Start a transaction response timer
 *
 * @param[in] thiz Pointer to stack instance
 * @param[in] timeoutArg Arg to timeout function.
 * @param[out] phTmr Pointer to timer handle. 
 * @return NW_IPv4_OK on success.
 */

NwIpv4RcT
nwIpv4StartTrxnPeerRspTimer(NwIpv4StackT* thiz, NwIpv4TimeoutInfoT* timeoutInfo, NwIpv4TimerHandleT* phTmr);

/**
 * Stop a transaction response timer
 *
 * @param[in] thiz Pointer to stack instance
 * @param[out] phTmr Pointer to timer handle. 
 * @return NW_IPv4_OK on success.
 */

NwIpv4RcT
nwIpv4StopTrxnPeerRspTimer(NwIpv4StackT* thiz, NwIpv4TimerHandleT* phTmr);


#define NW_IPv4_MAX_MSG_LEN                                    (4096)  /**< Maximum supported packet length including header */

/**
 * NwIpv4MsgT holds messages to/from the network.
 */
typedef struct NwIpv4Msg
{
  struct NwIpv4Msg *next;
  NwU16T           msgLen;
  NwU8T            msgBuf[NW_IPv4_MAX_MSG_LEN];
} NwIpv4MsgT;


/**
 * Transaction structure
 */

typedef struct NwIpv4Trxn
{
  NwU32T                        seqNum;
  NwU32T                        peerIp;
  NwU32T                        peerPort;
  NwU8T                         maxRetries;
  NwU8T                         t3Timer;
  NwIpv4TimerHandleT          hRspTmr;
  NwIpv4TimeoutInfoT          peerRspTimeoutInfo;
  NwIpv4StackT*               pStack;
  NwIpv4TunnelEndPointT*      pSession;
  NwU32T                        hUlpTrxn;
  NwIpv4MsgT*                 pMsg;
  //RB_ENTRY (NwIpv4Trxn)       outstandingTxSeqNumMapRbtNode;            /**< RB Tree Data Structure Node        */
  //RB_ENTRY (NwIpv4Trxn)       outstandingRxSeqNumMapRbtNode;            /**< RB Tree Data Structure Node        */
} NwIpv4TrxnT;


/**
 * IPv4 message header structure
 */

#pragma pack(1)

typedef struct NwIpv4MsgHeader 
{
  NwU8T PN:1;
  NwU8T S:1;
  NwU8T E:1;
  NwU8T spare:1;
  NwU8T PT:1;
  NwU8T version:3;
  NwU8T msgType;
  NwU16T msgLength;
  NwU32T ipv4Addr;
} NwIpv4MsgHeaderT;

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif  /* __NW_IPv4_PRIVATE_H__ */
/*--------------------------------------------------------------------------*
 *                      E N D     O F    F I L E                            *
 *--------------------------------------------------------------------------*/


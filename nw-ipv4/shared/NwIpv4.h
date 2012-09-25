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

#ifndef __NW_IPv4_H__
#define __NW_IPv4_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "NwTypes.h"
#include "NwIpv4Error.h"

/** @mainpage

  @section intro Introduction

  nw-Ipv4 library is a free and open source implementation of GPRS Tunneling
  protocol v2 user plane also known as eGTP-U based on 3GPP TS 29.281 V9.3.0. 
  The library is published under BSD three clause license.

  @section scope Scope

  The stack library also does basic tasks like packet/header validation, 
  retransmission, duplicate detection and message parsing.

  @section design Design Philosophy 

  The stack is fully-asynchronous in design for compatibility with event loop
  mechanisms such as select, poll, etc. and can also be used for multi-threaded 
  applications. It should compile on Linux, *BSD, Mac OS X, Solaris and Windows 
  (cygwin).

  The stack is designed for high portability not only for the hardware and OS it will 
  run on but also for the application software that uses it. The stack doesn't mandate
  conditions on the user application architecture or design. The stack relies on 
  the user application for infrastructure utilities such as I/O, timers, 
  logs and multithreading. This realized by using callback mechanisms and enables the
  stack library to seamlessly integrate without or very little changes to the existing 
  application framework. 

  The stack architecture builds upon following mentioned entities that are external to it.

  User Layer Protocol (ULP) Entity:
  This layer implements the intelligent logic for the application and sits on top of the
  stack. 

  LLP Entity:
  This is the layer below the stack and is responsible for I/O with the stack and network.
  It may or may not be housed in ULP. 

  Timer Manager Entity: 
  Timer Manager Entity provides the stack with infrastructure for timer CRUD operations. 

  Log Manager Entity:
  Log Manager Entity provides the stack with callbacks for logging operations. It may 
  or may not be housed in ULP. 

  The application may implement all above entities as a single or multiple object.

  @section applications Applications and Usage

  Please refer sample applications under 'test-app' directory for usage examples.

 */

/**
 * @file NwIpv4.h
 * @author Amit Chawre
 * @brief 
 *
 * This header file contains all required definitions and functions
 * prototypes for using nw-Ipv4 library. 
 *
 **/

#define NW_IPv4_VERSION                                                 (0x00)
#define NW_PROTOCOL_TYPE_IPv4                                           (0x800)
#define NW_IPv4_MODE_UPLINK                                             (0)/*< Required in case of uplink data emulator for ex. eNodeB emulator */
#define NW_IPv4_MODE_DOWNLINK                                           (1)/*< Normal case. All data from PDN to UE */
  
/*--------------------------------------------------------------------------*
 *                   S H A R E D     A P I    M A C R O S                   *
 *--------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * Opaque Ipv4 Stack Handles
 *--------------------------------------------------------------------------*/

typedef NwPtrT  NwIpv4StackHandleT;                             /**< Ipv4 Stack Handle                  */
typedef NwPtrT  NwIpv4UlpHandleT;                               /**< Ipv4 Stack Ulp Entity Handle       */ 
typedef NwPtrT  NwIpv4LlpHandleT;                               /**< Ipv4 Stack Udp Entity Handle       */ 
typedef NwPtrT  NwIpv4MemMgrHandleT;                            /**< Ipv4 Stack Mem Manager Handle      */ 
typedef NwPtrT  NwIpv4TimerMgrHandleT;                          /**< Ipv4 Stack Timer Manager Handle    */ 
typedef NwPtrT  NwIpv4LogMgrHandleT;                            /**< Ipv4 Stack Log Manager Handle      */ 
typedef NwPtrT  NwIpv4TimerHandleT;                             /**< Ipv4 Stack Timer Handle            */
typedef NwPtrT  NwIpv4MsgHandleT;                               /**< Ipv4 Msg Handle                    */

typedef struct NwIpv4StackConfig
{
  NwU16T __tbd;
} NwIpv4StackConfigT;

/*--------------------------------------------------------------------------*
 *            S T A C K        A P I      D E F I N I T I O N S             *
 *--------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * Ipv4 Stack ULP API type definitions
 *--------------------------------------------------------------------------*/

/** 
 * APIs types between ULP and Stack 
 */

typedef enum 
{
  /* APIs from ULP to stack */

  NW_IPv4_ULP_API_CREATE_TUNNEL_ENDPOINT = 0x00000000,          /**< Create a local IPv4 context on stack         */
  NW_IPv4_ULP_API_DESTROY_TUNNEL_ENDPOINT,                      /**< Delete a local IPv4 context on stack         */
  NW_IPv4_ULP_API_SEND_TPDU,                                    /**< Send a T-PDU message over IPv4 context       */

  /* APIs from stack to ULP */

  NW_IPv4_ULP_API_RECV_TPDU,                                    /**< Receive a Ipv4 T-PDU from stack              */
  NW_IPv4_ULP_API_RECV_MSG,                                     /**< Receive a ipv4 message from stack            */
  NW_IPv4_ULP_API_RSP_FAILURE,                                  /**< Rsp failure for ipv4 message from stack      */

  /* Do not add below this */

  NW_IPv4_ULP_API_END            = 0xFFFFFFFF,
} NwIpv4UlpApiTypeT;

/*---------------------------------------------------------------------------
 * Ipv4 Stack API information elements definitions
 *--------------------------------------------------------------------------*/

typedef NwPtrT NwIpv4StackSessionHandleT;                      /**< Ipv4 Stack session Handle                  */
typedef NwPtrT NwIpv4TrxnHandleT;                              /**< Ipv4 Transaction Handle                    */
typedef NwPtrT NwIpv4UlpTrxnHandleT;                           /**< Ipv4 Ulp Transaction Handle                */
typedef NwPtrT NwIpv4UlpSessionHandleT;                        /**< Ipv4 Ulp session Handle                    */

typedef NwU8T   NwIpv4MsgTypeT;                                 /**< Ipv4 Msg Type                              */

/** 
 * API information elements between ULP and Stack for 
 * creating a session. 
 */

typedef struct 
{
  NW_IN    NwU32T                       ipv4Addr; 
  NW_IN    NwIpv4UlpSessionHandleT    hUlpSession; 
  NW_OUT   NwIpv4StackSessionHandleT  hStackSession; 
} NwIpv4CreateTunnelEndPointT;

/** 
 * API information elements between ULP and Stack for 
 * destroying a session. 
 */

typedef struct 
{
  NW_IN   NwIpv4StackSessionHandleT   hStackSessionHandle; 
} NwIpv4DestroyTunnelEndPointT;

/** 
 * API information elements between ULP and Stack for 
 * sending a Ipv4 message over a session. 
 */

typedef struct 
{
  NW_IN    NwU32T                       ipv4Addr;
  NW_IN    NwU32T                       ipAddr;
  NW_IN    NwU8T                        flags;
  NW_IN    NwIpv4MsgHandleT             hMsg;
} NwIpv4SendtoInfoT;


/** 
 * API information elements between ULP and Stack for 
 * receiving a Ipv4 message over a session from stack. 
 */

typedef struct
{
  NW_IN    NwIpv4UlpSessionHandleT      hUlpSession; 
  NW_IN    NwIpv4MsgHandleT             hMsg;                   /**< IPv4 Message handle                 */
} NwIpv4RecvMsgInfoT;

/*---------------------------------------------------------------------------
 * Ipv4 Stack API structure definition
 *--------------------------------------------------------------------------*/

/** 
 * API structure between ULP and Stack 
 */

typedef struct 
{
  NwIpv4UlpApiTypeT             apiType;
  NwIpv4RcT                     rc;
  union 
  {
    NwIpv4CreateTunnelEndPointT       createTunnelEndPointInfo;
    NwIpv4DestroyTunnelEndPointT      destroyTunnelEndPointInfo;
    NwIpv4SendtoInfoT                 sendtoInfo;
    NwIpv4RecvMsgInfoT                recvMsgInfo;
  } apiInfo;
} NwIpv4UlpApiT;


/*--------------------------------------------------------------------------*
 *           S T A C K    E N T I T I E S    D E F I N I T I O N S          *
 *--------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * ULP Entity Definitions
 *--------------------------------------------------------------------------*/

/**
 * Ipv4 ULP entity definition
 */

typedef struct 
{
  NwIpv4UlpHandleT        hUlp;
  NwIpv4RcT (*ulpReqCallback) ( NW_IN        NwIpv4UlpHandleT hUlp, 
                            NW_IN        NwIpv4UlpApiT *pUlpApi);
} NwIpv4UlpEntityT;


/*---------------------------------------------------------------------------
 * LLP Entity Definitions
 *--------------------------------------------------------------------------*/

/**
 * Ipv4 LLP entity definition
 */

typedef struct
{
  NwIpv4LlpHandleT              hLlp;
  NwU8T                         llpHwAddr[6]; /* Ethernet Address */
  NwIpv4RcT (*llpDataReqCallback) ( 
      NW_IN     NwIpv4LlpHandleT llpHandle, 
      NW_IN     NwU8T*           dataBuf, 
      NW_IN     NwU32T           dataSize);
  NwIpv4RcT (*llpArpDataReqCallback) (
      NW_IN     NwIpv4LlpHandleT llpHandle, 
      NW_IN     NwU16T           opCode,
      NW_IN     NwU8T            *pTargetMac,
      NW_IN     NwU8T            *pTargetIpAddr,
      NW_IN     NwU8T            *pSenderIpAddr);

} NwIpv4LlpEntityT;

/**
 * Gtpv2c Memory Manager entity definition
 */

typedef struct
{
  NwIpv4MemMgrHandleT         hMemMgr;
  void* (*memAlloc)( NW_IN      NwIpv4MemMgrHandleT hMemMgr,
      NW_IN      NwU32T memSize,
      NW_IN      NwCharT* fileName,
      NW_IN      NwU32T lineNumber);

  void (*memFree) ( NW_IN       NwIpv4MemMgrHandleT hMemMgr,
      NW_IN       void* hMem,
      NW_IN       NwCharT* fileName,
      NW_IN       NwU32T lineNumber);
} NwIpv4MemMgrEntityT;

/*---------------------------------------------------------------------------
 * Timer Entity Definitions
 *--------------------------------------------------------------------------*/

#define NW_IPv4_TMR_TYPE_ONE_SHOT                               (0)
#define NW_IPv4_TMR_TYPE_REPETITIVE                             (1)

/**
 * Ipv4 Timer Manager entity definition
 */

typedef struct 
{
  NwIpv4TimerMgrHandleT        tmrMgrHandle;
  NwIpv4RcT (*tmrStartCallback)( NW_IN       NwIpv4TimerMgrHandleT tmrMgrHandle, 
                             NW_IN       NwU32T timeoutSecs,
                             NW_IN       NwU32T timeoutUsec,
                             NW_IN       NwU32T tmrType, 
                             NW_IN       void* tmrArg, 
                             NW_OUT      NwIpv4TimerHandleT* tmrHandle);

  NwIpv4RcT (*tmrStopCallback) ( NW_IN       NwIpv4TimerMgrHandleT tmrMgrHandle, 
                             NW_IN       NwIpv4TimerHandleT tmrHandle);
} NwIpv4TimerMgrEntityT;


/*---------------------------------------------------------------------------
 * Log Entity Definitions
 *--------------------------------------------------------------------------*/

/*
 * Log Level Definitions
 */

#define NW_LOG_LEVEL_EMER                                       (0) /**< system is unusable              */
#define NW_LOG_LEVEL_ALER                                       (1) /**< action must be taken immediately*/
#define NW_LOG_LEVEL_CRIT                                       (2) /**< critical conditions             */
#define NW_LOG_LEVEL_ERRO                                       (3) /**< error conditions                */
#define NW_LOG_LEVEL_WARN                                       (4) /**< warning conditions              */ 
#define NW_LOG_LEVEL_NOTI                                       (5) /**< normal but signification condition */
#define NW_LOG_LEVEL_INFO                                       (6) /**< informational                   */
#define NW_LOG_LEVEL_DEBG                                       (7) /**< debug-level messages            */

/**
 * Log Level Strings
 */

extern
NwCharT* ipv4LogLevelStr[];  

/**
 * Ipv4 Log manager entity definition
 */

typedef struct
{
  NwIpv4LogMgrHandleT          logMgrHandle;
  NwIpv4RcT (*logReqCallback) (NW_IN      NwIpv4LogMgrHandleT logMgrHandle, 
                           NW_IN      NwU32T logLevel,
                           NW_IN      NwCharT* file,
                           NW_IN      NwU32T line,
                           NW_IN      NwCharT* logStr);
} NwIpv4LogMgrEntityT;


/*--------------------------------------------------------------------------*
 *                     P U B L I C   F U N C T I O N S                      *
 *--------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif
    
/*---------------------------------------------------------------------------
 *  Constructor
 *--------------------------------------------------------------------------*/

/** 
 Initialize the nw-Ipv4 stack.

 @param[in,out] phIpv4StackHandle : Pointer to stack handle
 */

NwIpv4RcT
nwIpv4Initialize( NW_INOUT NwIpv4StackHandleT* phIpv4StackHandle);

/*---------------------------------------------------------------------------
 * Destructor
 *--------------------------------------------------------------------------*/

/** 
 Destroy the nw-Ipv4 stack.

 @param[in] hIpv4StackHandle : Stack handle
 */

NwIpv4RcT
nwIpv4Finalize( NW_IN  NwIpv4StackHandleT hIpv4StackHandle);

/*---------------------------------------------------------------------------
 * Configuration Get/Set Operations
 *--------------------------------------------------------------------------*/

/** 
 Set Configuration for the nw-Ipv4 stack.

 @param[in,out] phIpv4StackHandle : Pointer to stack handle
 */

NwIpv4RcT
NwIpv4ConfigSet( NW_IN NwIpv4StackHandleT* phIpv4StackHandle, NW_IN NwIpv4StackConfigT* pConfig);

/** 
 Get Configuration for the nw-Ipv4 stack.

 @param[in,out] phIpv4StackHandle : Pointer to stack handle
 */

NwIpv4RcT
NwIpv4ConfigGet( NW_IN NwIpv4StackHandleT* phIpv4StackHandle, NW_OUT NwIpv4StackConfigT* pConfig);

/** 
 Set mode for the stack.

 @param[in] hIpv4StackHandle : Stack handle
 @param[in] mode: uplink or downlink mode.
 @return NW_IPv4_OK on success.
 */

NwIpv4RcT
nwIpv4SetMode( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
               NW_IN NwU32T             mode);

/** 
 Set ULP entity for the stack.

 @param[in] hIpv4StackHandle : Stack handle
 @param[in] pUlpEntity : Pointer to ULP entity.
 @return NW_IPv4_OK on success.
 */

NwIpv4RcT
nwIpv4SetUlpEntity( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
                   NW_IN NwIpv4UlpEntityT* pUlpEntity);

/** 
 Set LLP entity for the stack.

 @param[in] hIpv4StackHandle : Stack handle
 @param[in] pLlpEntity : Pointer to LLP entity.
 @return NW_IPv4_OK on success.
 */

NwIpv4RcT
nwIpv4SetLlpEntity( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
                   NW_IN NwIpv4LlpEntityT* pLlpEntity);

/** 
 Set MemMgr entity for the stack.

 @param[in] hIpv4StackHandle : Stack handle
 @param[in] pMemMgr : Pointer to Mem Manager.
 @return NW_IPv4_OK on success.
 */

NwIpv4RcT
nwIpv4SetMemMgrEntity( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
                        NW_IN NwIpv4MemMgrEntityT* pMemMgr);

/** 
 Set TmrMgr entity for the stack.

 @param[in] hIpv4StackHandle : Stack handle
 @param[in] pTmrMgr : Pointer to Timer Manager.
 @return NW_IPv4_OK on success.
 */

NwIpv4RcT
nwIpv4SetTimerMgrEntity( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
                        NW_IN NwIpv4TimerMgrEntityT* pTmrMgr);

/** 
 Set LogMgr entity for the stack.

 @param[in] hIpv4StackHandle : Stack handle
 @param[in] pLogMgr : Pointer to Log Manager.
 @return NW_IPv4_OK on success.
 */

NwIpv4RcT
nwIpv4SetLogMgrEntity( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
                      NW_IN NwIpv4LogMgrEntityT* pLogMgr);

/** 
 Set log level for the stack.

 @param[in] hIpv4StackHandle : Stack handle
 @param[in] logLevel : Log level.
 @return NW_IPv4_OK on success.
 */

NwIpv4RcT
nwIpv4SetLogLevel( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
                     NW_IN NwU32T logLevel);
/*---------------------------------------------------------------------------
 * Process Request from LLP Layer
 *--------------------------------------------------------------------------*/

/**
 Process Data Request from LLP entity.

 @param[in] hIpv4StackHandle : Stack handle
 @param[in] data : Pointer to received UDP data.
 @param[in] dataLen : Received data length.
 @return NW_IPv4_OK on success.
 */

NwIpv4RcT 
nwIpv4ProcessLlpDataInd( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
                    NW_IN NwU8T* data,
                    NW_IN NwU32T dataLen);

/*---------------------------------------------------------------------------
 * Process Request from Upper Layer
 *--------------------------------------------------------------------------*/

/**
 Process Request from ULP entity.

 @param[in] hIpv4StackHandle : Stack handle
 @param[in] pLogMgr : Pointer to Ulp Req.
 @return NW_IPv4_OK on success.
 */

NwIpv4RcT
nwIpv4ProcessUlpReq( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
                       NW_IN NwIpv4UlpApiT *ulpReq);


/*---------------------------------------------------------------------------
 * Process Timer timeout Request from Timer Manager
 *--------------------------------------------------------------------------*/

/**
 Process Timer timeout Request from Timer Manager

 @param[in] pLogMgr : Pointer timeout arguments.
 @return NW_IPv4_OK on success.
 */

NwIpv4RcT
nwIpv4ProcessTimeout( NW_IN void* timeoutArg);

#ifdef __cplusplus
}
#endif

#endif  /* __NW_IPv4_H__ */

/*--------------------------------------------------------------------------*
 *                      E N D     O F    F I L E                            *
 *--------------------------------------------------------------------------*/


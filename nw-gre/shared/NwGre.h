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

#ifndef __NW_GRE_H__
#define __NW_GRE_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "NwTypes.h"
#include "NwGreError.h"

/** @mainpage

  @section intro Introduction

  nw-gre library is a free and open source implementation of GPRS Tunneling
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

  UDP Entity:
  This is the layer below the stack and is responsible for UDP I/O with the stack and network.
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
 * @file NwGre.h
 * @author Amit Chawre
 * @brief 
 *
 * This header file contains all required definitions and functions
 * prototypes for using nw-gre library. 
 *
 **/

#define NW_GRE_VERSION                                          (0x00)
#define NW_PROTOCOL_TYPE_IPv4                                   (0x800)
  
/*--------------------------------------------------------------------------*
 *                   S H A R E D     A P I    M A C R O S                   *
 *--------------------------------------------------------------------------*/

#define NW_GTP_ECHO_REQ                                         (1)
#define NW_GTP_ECHO_RSP                                         (2)
#define NW_GTP_ERROR_INDICATION                                 (26)
#define NW_GTP_SUPPORTED_EXTENSION_HEADER_INDICATION            (31)
#define NW_GTP_END_MARKER                                       (254)
#define NW_GTP_GPDU                                             (255)

/*---------------------------------------------------------------------------
 * Opaque Gre Stack Handles
 *--------------------------------------------------------------------------*/

typedef NwPtrT  NwGreStackHandleT;                     /**< Gre Stack Handle                    */
typedef NwPtrT  NwGreUlpHandleT;                       /**< Gre Stack Ulp Entity Handle         */ 
typedef NwPtrT  NwGreUdpHandleT;                       /**< Gre Stack Udp Entity Handle         */ 
typedef NwPtrT  NwGreMemMgrHandleT;                    /**< Gre Stack Memory Manager Handle     */ 
typedef NwPtrT  NwGreTimerMgrHandleT;                  /**< Gre Stack Timer Manager Handle      */ 
typedef NwPtrT  NwGreLogMgrHandleT;                    /**< Gre Stack Log Mnagaer Handle        */ 
typedef NwPtrT  NwGreTimerHandleT;                     /**< Gre Stack Timer Handle              */
typedef NwPtrT  NwGreMsgHandleT;                       /**< Gre Msg Handle                      */

typedef struct NwGreStackConfig
{
  NwU16T udpSrcPort;
} NwGreStackConfigT;

/*--------------------------------------------------------------------------*
 *            S T A C K        A P I      D E F I N I T I O N S             *
 *--------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * Gre Stack ULP API type definitions
 *--------------------------------------------------------------------------*/

/** 
 * APIs types between ULP and Stack 
 */

typedef enum 
{
  /* APIs from ULP to stack */

  NW_GRE_ULP_API_CREATE_TUNNEL_ENDPOINT = 0x00000000,        /**< Create a local tunnel context on stack        */
  NW_GRE_ULP_API_DESTROY_TUNNEL_ENDPOINT,                    /**< Delete a local tunnel context on stack        */
  NW_GRE_ULP_API_INITIAL_REQ,                                /**< Send a Initial Request over a session         */
  NW_GRE_ULP_API_TRIGGERED_REQ,                              /**< Send a Initial Request over a session         */
  NW_GRE_ULP_API_TRIGGERED_RSP,                              /**< Send a Trigger Response over a session        */
  NW_GRE_ULP_API_SEND_TPDU,                                  /**< Send a T-PDU message over tunnel context      */

  /* APIs from stack to ULP */

  NW_GRE_ULP_API_RECV_TPDU,                                  /**< Receive a gre T-PDU from stack                */
  NW_GRE_ULP_API_RECV_MSG,                                   /**< Receive a gre message from stack              */
  NW_GRE_ULP_API_RSP_FAILURE,                                /**< Rsp failure for gre message from stack        */

  /* Do not add below this */

  NW_GRE_ULP_API_END            = 0xFFFFFFFF,
} NwGreUlpApiTypeT;

/*---------------------------------------------------------------------------
 * Gre Stack API information elements definitions
 *--------------------------------------------------------------------------*/

typedef NwPtrT  NwGreStackSessionHandleT;/**< Gre Stack session Handle */
typedef NwPtrT  NwGreTrxnHandleT;        /**< Gre Transaction Handle */
typedef NwPtrT  NwGreUlpTrxnHandleT;     /**< Gre Ulp Transaction Handle */
typedef NwPtrT  NwGreUlpSessionHandleT;  /**< Gre Ulp session Handle */

typedef NwU8T   NwGreMsgTypeT;           /**< Gre Msg Type     */
/** 
 * API information elements between ULP and Stack for 
 * creating a session. 
 */

typedef struct 
{
  NW_IN    NwU32T                    greKey; 
  NW_IN    NwGreUlpSessionHandleT    hUlpSession; 
  NW_OUT   NwGreStackSessionHandleT  hStackSession; 
} NwGreCreateTunnelEndPointT;

/** 
 * API information elements between ULP and Stack for 
 * destroying a session. 
 */

typedef struct 
{
  NW_IN   NwGreStackSessionHandleT   hStackSessionHandle; 
} NwGreDestroyTunnelEndPointT;

/** 
 * API information elements between ULP and Stack for 
 * sending a Gre initial message. 
 */

typedef struct 
{
  NW_IN    NwGreUlpTrxnHandleT       hUlpTrxn;
  NW_IN    NwU32T                    peerIp;
  NW_IN    NwU32T                    peerPort;
  NW_IN    NwU8T                     flags;
  NW_IN    NwU32T                    greKey;
  NW_IN    NwGreMsgHandleT           hMsg;
} NwGreInitialReqInfoT;

/** 
 * API information elements between ULP and Stack for 
 * sending a Gre triggered response message. 
 */

typedef struct 
{
  NW_IN    NwGreUlpTrxnHandleT        hUlpTrxn;
  NW_IN    NwU32T                        peerIp;
  NW_IN    NwU32T                        peerPort;
  NW_IN    NwU8T                         flags;
  NW_IN    NwU32T                        greKey;
  NW_IN    NwU32T                        seqNum;
  NW_IN    NwGreMsgHandleT            hMsg;
} NwGreTriggeredRspInfoT;

/** 
 * API information elements between ULP and Stack for 
 * sending a Gre triggered request message. 
 */

typedef struct 
{
  NW_IN    NwGreUlpTrxnHandleT        hUlpTrxn;
  NW_IN    NwU32T                        peerIp;
  NW_IN    NwU32T                        peerPort;
  NW_IN    NwU8T                         flags;
  NW_IN    NwU32T                        greKey;
  NW_IN    NwU32T                        seqNum;
  NW_IN    NwGreMsgHandleT            hMsg;
} NwGreTriggeredReqInfoT;


/** 
 * API information elements between ULP and Stack for 
 * sending a Gre message over a session. 
 */

typedef struct 
{
  NW_IN    NwU32T                       greKey;
  NW_IN    NwU32T                       ipAddr;
  NW_IN    NwU8T                        flags;
  NW_IN    NwGreMsgHandleT           hMsg;
} NwGreSendtoInfoT;


/** 
 * API information elements between ULP and Stack for 
 * sending a Gre message over a session. 
 */

typedef struct 
{
  NW_OUT   NwGreStackSessionHandleT  hStackSessionHandle; 
  NW_INOUT NwGreTrxnHandleT          hTrxn;
  NW_IN    NwGreUlpTrxnHandleT       hUlpTrxn;
  NW_IN    NwGreMsgTypeT            msgType;
  NW_IN    NwU8T                        flags;
  NW_IN    NwGreMsgHandleT           hMsg;
} NwGreSendMsgInfoT;

/** 
 * API information elements between ULP and Stack for 
 * receiving a Gre message over a session from stack. 
 */

typedef struct
{
  NW_IN    NwGreUlpSessionHandleT    hUlpSession; 
  NW_IN    NwGreUlpTrxnHandleT       hUlpTrxn;
  NW_IN    NwU32T                    greKey;
  NW_IN    NwU32T                    peerIp;
  NW_IN    NwU32T                    peerPort;
  NW_IN    NwU32T                    msgType;      /**< Message type                       */
  NW_IN    NwGreMsgHandleT           hMsg;         /**< GRE Message handle                 */
} NwGreRecvMsgInfoT;

/** 
 * API information elements between ULP and Stack for 
 * receiving a Gre message over a session from stack. 
 */

typedef struct
{
  NW_IN    NwGreUlpSessionHandleT    hUlpSession; 
  NW_IN    NwGreTrxnHandleT          hTrxn;
} NwGreNackInfoT;

/*---------------------------------------------------------------------------
 * Gre Stack API structure definition
 *--------------------------------------------------------------------------*/

/** 
 * API structure between ULP and Stack 
 */

typedef struct 
{
  NwGreUlpApiTypeT               apiType;
  NwRcT rc;
  union 
  {
    NwGreCreateTunnelEndPointT       createTunnelEndPointInfo;
    NwGreDestroyTunnelEndPointT      destroyTunnelEndPointInfo;
    NwGreInitialReqInfoT             initialReqInfo;
    NwGreTriggeredRspInfoT           triggeredRspInfo;
    NwGreTriggeredReqInfoT           triggeredReqInfo;
    NwGreSendtoInfoT                 sendtoInfo;
    NwGreSendMsgInfoT                sendMsgInfo;
    NwGreRecvMsgInfoT                recvMsgInfo;
    NwGreNackInfoT                   nackMsgInfo;
  } apiInfo;
} NwGreUlpApiT;


/*--------------------------------------------------------------------------*
 *           S T A C K    E N T I T I E S    D E F I N I T I O N S          *
 *--------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * ULP Entity Definitions
 *--------------------------------------------------------------------------*/

/**
 * Gre ULP entity definition
 */

typedef struct 
{
  NwGreUlpHandleT        hUlp;
  NwRcT (*ulpReqCallback) ( NW_IN        NwGreUlpHandleT hUlp, 
                            NW_IN        NwGreUlpApiT *pUlpApi);
} NwGreUlpEntityT;


/*---------------------------------------------------------------------------
 * UDP Entity Definitions
 *--------------------------------------------------------------------------*/

/**
 * Gre UDP entity definition
 */

typedef struct
{
  NwGreUdpHandleT        hUdp;
  NwRcT (*udpDataReqCallback) ( NW_IN     NwGreUdpHandleT udpHandle, 
                                NW_IN     NwU8T* dataBuf, 
                                NW_IN     NwU32T dataSize,
                                NW_IN     NwU32T peerIP,
                                NW_IN     NwU32T peerPort);
} NwGreLlpEntityT;

/**
 * Memory Manager entity definition
 */
  
typedef struct
{ 
  NwGreMemMgrHandleT         hMemMgr;
  void* (*memAlloc)( NW_IN      NwGreMemMgrHandleT hMemMgr, 
      NW_IN      NwU32T memSize,
      NW_IN      NwCharT* fileName,
      NW_IN      NwU32T lineNumber);

  void (*memFree) ( NW_IN       NwGreMemMgrHandleT hMemMgr,
      NW_IN       void* hMem,
      NW_IN       NwCharT* fileName,
      NW_IN       NwU32T lineNumber);
} NwGreMemMgrEntityT;


/*---------------------------------------------------------------------------
 * Timer Entity Definitions
 *--------------------------------------------------------------------------*/

#define NW_GRE_TMR_TYPE_ONE_SHOT                                  (0)
#define NW_GRE_TMR_TYPE_REPETITIVE                                (1)

/**
 * Gre Timer Manager entity definition
 */

typedef struct 
{
  NwGreTimerMgrHandleT        tmrMgrHandle;
  NwRcT (*tmrStartCallback)( NW_IN       NwGreTimerMgrHandleT tmrMgrHandle, 
                             NW_IN       NwU32T timeoutSecs,
                             NW_IN       NwU32T timeoutUsec,
                             NW_IN       NwU32T tmrType, 
                             NW_IN       void* tmrArg, 
                             NW_OUT      NwGreTimerHandleT* tmrHandle);

  NwRcT (*tmrStopCallback) ( NW_IN       NwGreTimerMgrHandleT tmrMgrHandle, 
                             NW_IN       NwGreTimerHandleT tmrHandle);
} NwGreTimerMgrEntityT;


/*---------------------------------------------------------------------------
 * Log Entity Definitions
 *--------------------------------------------------------------------------*/

/*
 * Log Level Definitions
 */

#define NW_LOG_LEVEL_EMER                       (0) /**< system is unusable              */
#define NW_LOG_LEVEL_ALER                       (1) /**< action must be taken immediately*/
#define NW_LOG_LEVEL_CRIT                       (2) /**< critical conditions             */
#define NW_LOG_LEVEL_ERRO                       (3) /**< error conditions                */
#define NW_LOG_LEVEL_WARN                       (4) /**< warning conditions              */ 
#define NW_LOG_LEVEL_NOTI                       (5) /**< normal but signification condition */
#define NW_LOG_LEVEL_INFO                       (6) /**< informational                   */
#define NW_LOG_LEVEL_DEBG                       (7) /**< debug-level messages            */

/**
 * Log Level Strings
 */

extern
NwCharT* greLogLevelStr[];  

/**
 * Gre Log manager entity definition
 */

typedef struct
{
  NwGreLogMgrHandleT          logMgrHandle;
  NwRcT (*logReqCallback) (NW_IN      NwGreLogMgrHandleT logMgrHandle, 
                           NW_IN      NwU32T logLevel,
                           NW_IN      NwCharT* file,
                           NW_IN      NwU32T line,
                           NW_IN      NwCharT* logStr);
} NwGreLogMgrEntityT;


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
 Initialize the nw-gre stack.

 @param[in,out] phGreStackHandle : Pointer to stack handle
 */

NwRcT
nwGreInitialize( NW_INOUT NwGreStackHandleT* phGreStackHandle);

/*---------------------------------------------------------------------------
 * Destructor
 *--------------------------------------------------------------------------*/

/** 
 Destroy the nw-gre stack.

 @param[in] hGreStackHandle : Stack handle
 */

NwRcT
nwGreFinalize( NW_IN  NwGreStackHandleT hGreStackHandle);

/*---------------------------------------------------------------------------
 * Configuration Get/Set Operations
 *--------------------------------------------------------------------------*/

/** 
 Set Configuration for the nw-gre stack.

 @param[in,out] phGreStackHandle : Pointer to stack handle
 */

NwRcT
NwGreConfigSet( NW_IN NwGreStackHandleT* phGreStackHandle, NW_IN NwGreStackConfigT* pConfig);

/** 
 Get Configuration for the nw-gre stack.

 @param[in,out] phGreStackHandle : Pointer to stack handle
 */

NwRcT
NwGreConfigGet( NW_IN NwGreStackHandleT* phGreStackHandle, NW_OUT NwGreStackConfigT* pConfig);

/** 
 Set ULP entity for the stack.

 @param[in] hGreStackHandle : Stack handle
 @param[in] pUlpEntity : Pointer to ULP entity.
 @return NW_OK on success.
 */

NwRcT
nwGreSetUlpEntity( NW_IN NwGreStackHandleT hGreStackHandle,
                   NW_IN NwGreUlpEntityT* pUlpEntity);

/** 
 Set UDP entity for the stack.

 @param[in] hGreStackHandle : Stack handle
 @param[in] pUdpEntity : Pointer to UDP entity.
 @return NW_OK on success.
 */

NwRcT
nwGreSetUdpEntity( NW_IN NwGreStackHandleT hGreStackHandle,
                   NW_IN NwGreLlpEntityT* pUdpEntity);

/** 
 Set TmrMgr entity for the stack.

 @param[in] hGreStackHandle : Stack handle
 @param[in] pTmrMgr : Pointer to Timer Manager.
 @return NW_OK on success.
 */

NwRcT
nwGreSetTimerMgrEntity( NW_IN NwGreStackHandleT hGreStackHandle,
                        NW_IN NwGreTimerMgrEntityT* pTmrMgr);

/** 
 Set LogMgr entity for the stack.

 @param[in] hGreStackHandle : Stack handle
 @param[in] pLogMgr : Pointer to Log Manager.
 @return NW_OK on success.
 */

NwRcT
nwGreSetLogMgrEntity( NW_IN NwGreStackHandleT hGreStackHandle,
                      NW_IN NwGreLogMgrEntityT* pLogMgr);

/** 
 Set log level for the stack.

 @param[in] hGreStackHandle : Stack handle
 @param[in] logLevel : Log level.
 @return NW_OK on success.
 */

NwRcT
nwGreSetLogLevel( NW_IN NwGreStackHandleT hGreStackHandle,
                     NW_IN NwU32T logLevel);
/*---------------------------------------------------------------------------
 * Process Request from Udp Layer
 *--------------------------------------------------------------------------*/

/**
 Process Data Request from UDP entity.

 @param[in] hGreStackHandle : Stack handle
 @param[in] udpData : Pointer to received UDP data.
 @param[in] udpDataLen : Received data length.
 @param[in] dstPort : Received on port.
 @param[in] from : Received from peer information.
 @return NW_OK on success.
 */

NwRcT 
nwGreProcessUdpReq( NW_IN NwGreStackHandleT hGreStackHandle,
                    NW_IN NwCharT* udpData,
                    NW_IN NwU32T udpDataLen,
                    NW_IN NwU16T peerPort,
                    NW_IN NwU32T peerIP);

/*---------------------------------------------------------------------------
 * Process Request from Upper Layer
 *--------------------------------------------------------------------------*/

/**
 Process Request from ULP entity.

 @param[in] hGreStackHandle : Stack handle
 @param[in] pLogMgr : Pointer to Ulp Req.
 @return NW_OK on success.
 */

NwRcT
nwGreProcessUlpReq( NW_IN NwGreStackHandleT hGreStackHandle,
                       NW_IN NwGreUlpApiT *ulpReq);


/*---------------------------------------------------------------------------
 * Process Timer timeout Request from Timer Manager
 *--------------------------------------------------------------------------*/

/**
 Process Timer timeout Request from Timer Manager

 @param[in] pLogMgr : Pointer timeout arguments.
 @return NW_OK on success.
 */

NwRcT
nwGreProcessTimeout( NW_IN void* timeoutArg);

#ifdef __cplusplus
}
#endif

#endif  /* __NW_GRE_H__ */

/*--------------------------------------------------------------------------*
 *                      E N D     O F    F I L E                            *
 *--------------------------------------------------------------------------*/


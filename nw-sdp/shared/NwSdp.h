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

#ifndef __NW_SDP_H__
#define __NW_SDP_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "NwTypes.h"
#include "NwSdpError.h"
#include "NwGtpv1u.h"

/** @mainpage

  @section intro Introduction

  @section scope Scope

  @section design Design Philosophy 

  @section applications Applications and Usage

 */

/**
 * @file NwSdp.h
 * @author Amit Chawre
 * @brief 
 *
 * This header file contains all required definitions and functions
 * prototypes for using nw-sdp software library. 
 *
 */

#define NW_SDP_VERSION                                                  (0x00)
#define NW_SDP_IPv4_MODE_UPLINK                                         (0)/*< Required in case of uplink data emulator for ex. eNodeB emulator */
#define NW_SDP_IPv4_MODE_DOWNLINK                                       (1)/*< Normal case. All data from PDN to UE */

  
/*---------------------------------------------------------------------------
 * Opaque Sdp Handles
 *--------------------------------------------------------------------------*/

typedef NwPtrT  NwSdpHandleT;                           /**< Sdp Handle                 */
typedef NwPtrT  NwSdpServiceHandleT;                    /**< Sdp Service Handle         */
typedef NwPtrT  NwSdpUlpHandleT;                        /**< Sdp Ulp Entity Handle      */ 
typedef NwPtrT  NwSdpUdpHandleT;                        /**< Sdp Udp Entity Handle      */ 
typedef NwPtrT  NwSdpTimerMgrHandleT;                   /**< Sdp Timer Manager Handle   */ 
typedef NwPtrT  NwSdpMemMgrHandleT;                     /**< Sdp Memory Manager Handle  */ 
typedef NwPtrT  NwSdpLogMgrHandleT;                     /**< Sdp Log Manager Handle     */ 
typedef NwPtrT  NwSdpTimerHandleT;                      /**< Sdp Timer Handle           */
typedef NwPtrT  NwSdpMsgHandleT;                        /**< Sdp Msg Handle             */

typedef struct NwSdpConfig
{
  NwU16T udpSrcPort;
  NwU32T __tbd;
} NwSdpConfigT;

/*--------------------------------------------------------------------------*
 *                S D P     A P I      D E F I N I T I O N S                *
 *--------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * Sdp ULP API type definitions
 *--------------------------------------------------------------------------*/

/** 
 * APIs types between ULP and Stack 
 */

typedef enum 
{
  /* APIs from ULP to SDP */

  NW_SDP_ULP_API_CREATE_FLOW = 0x00000000,              /**< Create a local flow context on SDP         */
  NW_SDP_ULP_API_DESTROY_FLOW,                          /**< Delete a local flow context on SDP         */
  NW_SDP_ULP_API_UPDATE_FLOW,                           /**< Update a local flow context on SDP         */

  /* Do not add below this */
  NW_SDP_ULP_API_END
} NwSdpUlpApiTypeT;

/*---------------------------------------------------------------------------
 * SdpAPI information elements definitions
 *--------------------------------------------------------------------------*/

typedef NwPtrT NwSdpSessionHandleT;                    /**< Sdpsession Handle                          */
typedef NwPtrT NwSdpUlpSessionHandleT;                 /**< SDP Ulp session Handle                     */
typedef NwU8T   NwSdpMsgTypeT;                          /**< SDP Msg Type                               */

typedef enum
{
  NW_SDP_SERVICE_TYPE_IPv4= 0x000000,                   /**< An IPv4 service type                       */
  NW_SDP_SERVICE_TYPE_GRE,                              /**< A SDP service type                         */
  NW_SDP_SERVICE_TYPE_GTPU,                             /**< A GTP-U service type                       */
  NW_SDP_SERVICE_TYPE_UDP,                              /**< A UDP service type                         */

  /* Do not add below this */
  NW_SDP_SERVICE_TYPE_END
} NwSdpServiceTypeT;


typedef enum
{
  NW_FLOW_TYPE_IPv4 = 0x000000,                         /**< An IPv4 Flow End Point Type                */
  NW_FLOW_TYPE_GRE,                                     /**< A GRE Flow End Point Type                  */
  NW_FLOW_TYPE_GTPU,                                    /**< A GTP-U Flow End Point Type                */
  NW_FLOW_TYPE_UDP,                                     /**< A UDP Flow End Point Type                  */

  /* Do not add below this */
  NW_FLOW_TYPE_END
} NwSdpFlowEndPointTypeT;

/** 
 * Flow definition
 */

typedef struct 
{
  NwU32T          ipv4Addr;
  NwSdpFlowEndPointTypeT  flowType;

  union {
    NwU32T        gtpuTeid;
    NwU32T        greKey;
    NwU32T        ipv4Addr;
    struct {
      NwU16T      port;
      NwU32T      ipv4Addr;
    } udp;
  } flowKey;

  union {
    NwU32T gtpu;
    NwU32T gre;
    NwU32T udp;
    NwU32T ipv4;
  } hTunnelEndPoint;

} NwSdpFlowEndPointT;

/** 
 * API information elements between ULP and Stack for 
 * creating a flow. 
 */

typedef struct 
{
  NW_IN   NwSdpFlowEndPointT           ingressEndPoint;
  NW_IN   NwSdpFlowEndPointT           egressEndPoint;
  NW_IN   NwSdpUlpSessionHandleT       hUlpSession; 
  NW_OUT  NwSdpSessionHandleT          hSdpSession; 
} NwSdpCreateFlowInfoT;

/** 
 * API information elements between ULP and Stack for 
 * destroying a flow. 
 */

typedef struct 
{
  NW_IN   NwSdpSessionHandleT          hSdpSession; 
} NwSdpDestroyFlowInfoT;

/** 
 * API information elements between ULP and Stack for 
 * updating a flow. 
 */

typedef struct 
{
  NW_IN   NwSdpFlowEndPointT            ingressFlow;
  NW_IN   NwSdpFlowEndPointT            egressFlow;
  NW_IN   NwSdpUlpSessionHandleT        hUlpSession; 
  NW_IN   NwSdpSessionHandleT           hSdpSession; 
} NwSdpUpdateFlowInfoT;

/** 
 * API information elements between ULP and Stack for 
 * sending a Gre message over a session. 
 */

typedef struct 
{
  NW_IN    NwU32T                       teid;
  NW_IN    NwU32T                       ipAddr;
  NW_IN    NwU8T                        flags;
  NW_IN    NwSdpMsgHandleT              hMsg;
} NwSdpSendtoInfoT;

/** 
 * API information elements between ULP and Stack for 
 * receiving a Gre message over a session from stack. 
 */

typedef struct
{
  NW_IN    NwSdpUlpSessionHandleT       hUlpSession; 
  NW_IN    NwU32T                       teid;
  NW_IN    NwU32T                       peerIp;
  NW_IN    NwU32T                       peerPort;
  NW_IN    NwU32T                       msgType;      /**< Message type                         */
  NW_IN    NwSdpMsgHandleT              hMsg;         /**< Message handle                       */
} NwSdpRecvMsgInfoT;


/*---------------------------------------------------------------------------
 * Sdp API structure definition
 *--------------------------------------------------------------------------*/

/** 
 * API structure between ULP and Stack 
 */

typedef struct 
{
  NwSdpUlpApiTypeT              apiType;
  NwSdpRcT rc;
  union 
  {
    NwSdpCreateFlowInfoT        createFlowInfo;
    NwSdpDestroyFlowInfoT       destroyFlowInfo;
    NwSdpUpdateFlowInfoT        updateFlowInfo;
  } apiInfo;
} NwSdpUlpApiT;


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
  NwSdpUlpHandleT        hUlp;
  NwSdpRcT (*ulpReqCallback) ( NW_IN        NwSdpUlpHandleT hUlp, 
                            NW_IN        NwSdpUlpApiT *pUlpApi);
} NwSdpUlpEntityT;


/*---------------------------------------------------------------------------
 * UDP Entity Definitions
 *--------------------------------------------------------------------------*/

/**
 * Gre UDP entity definition
 */

typedef struct
{
  NwSdpUdpHandleT        hUdp;
  NwSdpRcT (*udpDataReqCallback) ( NW_IN     NwSdpUdpHandleT udpHandle, 
                                NW_IN     NwU8T* dataBuf, 
                                NW_IN     NwU32T dataSize,
                                NW_IN     NwU32T peerIP,
                                NW_IN     NwU32T peerPort);
} NwSdpUdpEntityT;

/**
 * Memory Manager Entity definition
 */

typedef struct
{
  NwSdpMemMgrHandleT         hMemMgr;
  void* (*memAlloc)( NW_IN      NwSdpMemMgrHandleT hMemMgr,
      NW_IN      NwU32T memSize,
      NW_IN      NwCharT* fileName,
      NW_IN      NwU32T lineNumber);

  void (*memFree) ( NW_IN       NwSdpMemMgrHandleT hMemMgr,
      NW_IN       void* hMem,
      NW_IN       NwCharT* fileName,
      NW_IN       NwU32T lineNumber);
} NwSdpMemMgrEntityT;

#define NW_SDP_TMR_TYPE_ONE_SHOT                                  (0)
#define NW_SDP_TMR_TYPE_REPETITIVE                                (1)

/**
 * Timer Manager entity definition
 */

typedef struct 
{
  NwSdpTimerMgrHandleT        tmrMgrHandle;
  NwSdpRcT (*tmrStartCallback)( NW_IN       NwSdpTimerMgrHandleT tmrMgrHandle, 
                             NW_IN       NwU32T timeoutSecs,
                             NW_IN       NwU32T timeoutUsec,
                             NW_IN       NwU32T tmrType, 
                             NW_IN       void* tmrArg, 
                             NW_OUT      NwSdpTimerHandleT* tmrHandle);

  NwSdpRcT (*tmrStopCallback) ( NW_IN       NwSdpTimerMgrHandleT tmrMgrHandle, 
                             NW_IN       NwSdpTimerHandleT tmrHandle);
} NwSdpTimerMgrEntityT;


/*---------------------------------------------------------------------------
 * Log Entity Definitions
 *--------------------------------------------------------------------------*/

/**
 * Log manager entity definition
 */

typedef struct
{
  NwSdpLogMgrHandleT          logMgrHandle;
  NwSdpRcT (*logReqCallback) (NW_IN      NwSdpLogMgrHandleT logMgrHandle, 
                           NW_IN      NwU32T logLevel,
                           NW_IN      NwCharT* file,
                           NW_IN      NwU32T line,
                           NW_IN      NwCharT* logStr);
} NwSdpLogMgrEntityT;


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
 Initialize the soft date plane.

 @param[in,out] phSdp : Pointer to SDP handle
 */

NwSdpRcT
nwSdpInitialize( NW_INOUT NwSdpHandleT* phSdp);

/*---------------------------------------------------------------------------
 * Destructor
 *--------------------------------------------------------------------------*/

/** 
 Destroy the nw-sdp instance.

 @param[in] hSdp : SDP handle
 */

NwSdpRcT
nwSdpFinalize( NW_IN  NwSdpHandleT hSdp);


/** 
 Create GTPU service instance

 @param[in] hSdp : SDP handle.
 @param[in] hTlService: Transport Layer Service handle.
 @param[in] pTlDataReqCb: Transport Layer Service data request callback.
 @param[in] serviceCfg: Service configuration.
 @param[out] phSdpService: Pointer to SDP Service handle.
 @return NW_SDP_OK on success.
 */

NwSdpRcT
nwSdpCreateGtpuService( NW_IN NwSdpHandleT hSdp,
                        NW_IN NwU32T hGtpuTlInterface,
                        NW_IN NwSdpRcT (*pGtpuTlDataReqCb)( NwGtpv1uUdpHandleT hUdp,
                                                        NwU8T* dataBuf,
                                                        NwU32T dataSize,
                                                        NwU32T peerIpAddr,
                                                        NwU32T peerPort),
                       NW_IN NwSdpServiceHandleT* phSdpService);

/** 
 Create Ipv4 service instance

 @param[in] hSdp : SDP handle.
 @param[in] mode : Uplink or downlink mode.
 @param[in] hTlService: Transport Layer Service handle.
 @param[in] pTlDataReqCb: Transport Layer Service data request callback.
 @param[in] serviceCfg: Service configuration.
 @param[out] phSdpService: Pointer to SDP Service handle.
 @return NW_SDP_OK on success.
 */

NwSdpRcT
nwSdpCreateIpv4Service( NW_IN NwSdpHandleT      hSdp,
                        NW_IN NwU32T            mode,
                        NW_IN NwU8T             *pHwAddr,
                        NW_IN NwU32T            hIpv4TlInterface,
                        NW_IN NwSdpRcT          (*pIpv4TlDataReqCb)( NwU32T udpHandle,
                                                        NwU8T* dataBuf,
                                                        NwU32T dataSize),
                        NW_IN NwSdpRcT          (*pIpv4TlArpDataReqCb) (
                                                        NwU32T          udpHandle,       
                                                        NwU16T           opCode,
                                                        NwU8T            *pTargetMac,
                                                        NwU8T            *pTargetIpAddr,
                                                        NwU8T            *pSenderIpAddr),
                       NW_IN NwSdpServiceHandleT* phSdpService);

/** 
  Destroy a new SDP service instance

 @param[in] hSdp : SDP handle.
 @param[in] hGreInterface: GRE transport handle.
 @param[out] phSdpService: Pointer to SDP Service handle.
 @return NW_SDP_OK on success.
 */

NwSdpRcT
nwSdpDestroyService( NW_IN NwSdpHandleT hSdp,
                    NW_IN NwSdpServiceHandleT* phSdpService);


/*---------------------------------------------------------------------------
 * Configuration Get/Set Operations
 *--------------------------------------------------------------------------*/

/** 
 Set Configuration for the nw-sdp instance.

 @param[in,out] phSdp : Pointer to SDP handle
 */

NwSdpRcT
NwSdpConfigSet( NW_IN NwSdpHandleT* phSdp, NW_IN NwSdpConfigT* pConfig);

/** 
 Get Configuration for the nw-sdp instance.

 @param[in,out] phSdp : Pointer to SDP handle
 */

NwSdpRcT
NwSdpConfigGet( NW_IN NwSdpHandleT* phSdp, NW_OUT NwSdpConfigT* pConfig);

/** 
 Set ULP entity for the stack.

 @param[in] hSdp : SDP handle
 @param[in] pUlpEntity : Pointer to ULP entity.
 @return NW_SDP_OK on success.
 */

NwSdpRcT
nwSdpSetUlpEntity( NW_IN NwSdpHandleT hSdp,
                   NW_IN NwSdpUlpEntityT* pUlpEntity);

/** 
 Set UDP entity for the stack.

 @param[in] hSdp : SDP handle
 @param[in] pUdpEntity : Pointer to UDP entity.
 @return NW_SDP_OK on success.
 */

NwSdpRcT
nwSdpSetUdpEntity( NW_IN NwSdpHandleT hSdp,
                   NW_IN NwSdpUdpEntityT* pUdpEntity);

/** 
 Set MemMgr entity for the stack.

 @param[in] hSdp : SDP handle
 @param[in] pTmrMgr : Pointer to Memory Manager.
 @return NW_SDP_OK on success.
 */

NwSdpRcT
nwSdpSetMemMgrEntity( NW_IN NwSdpHandleT hSdp,
                        NW_IN NwSdpMemMgrEntityT* pMemMgr);


/** 
 Set TmrMgr entity for the stack.

 @param[in] hSdp : SDP handle
 @param[in] pTmrMgr : Pointer to Timer Manager.
 @return NW_SDP_OK on success.
 */

NwSdpRcT
nwSdpSetTimerMgrEntity( NW_IN NwSdpHandleT hSdp,
                        NW_IN NwSdpTimerMgrEntityT* pTmrMgr);

/** 
 Set LogMgr entity for the stack.

 @param[in] hSdp : SDP handle
 @param[in] pLogMgr : Pointer to Log Manager.
 @return NW_SDP_OK on success.
 */

NwSdpRcT
nwSdpSetLogMgrEntity( NW_IN NwSdpHandleT hSdp,
                      NW_IN NwSdpLogMgrEntityT* pLogMgr);

/** 
 Set log level for the stack.

 @param[in] hSdp : SDP handle
 @param[in] logLevel : Log level.
 @return NW_SDP_OK on success.
 */

NwSdpRcT
nwSdpSetLogLevel( NW_IN NwSdpHandleT hSdp,
                     NW_IN NwU32T logLevel);
/*---------------------------------------------------------------------------
 * Process Request from Udp Layer
 *--------------------------------------------------------------------------*/

/**
 Process Data Request from UDP entity.

 @param[in] hSdp : SDP handle
 @param[in] udpData : Pointer to received UDP data.
 @param[in] udpDataLen : Received data length.
 @param[in] peerPort : Received on port.
 @param[in] peerIp : Received from IP.
 @return NW_SDP_OK on success.
 */

NwSdpRcT 
nwSdpProcessUdpDataInd( NW_IN NwSdpHandleT hSdp,
                    NW_IN NwU8T* udpData,
                    NW_IN NwU32T udpDataLen,
                    NW_IN NwU16T peerPort,
                    NW_IN NwU32T peerIP);

NwSdpRcT 
nwSdpProcessIpv4DataInd( NW_IN NwSdpHandleT hSdp, 
                    NW_IN NwSdpServiceHandleT hIpv4,
                    NW_IN NwU8T* ipv4Buf,
                    NW_IN NwU32T ipv4BufLen);

NwSdpRcT 
nwSdpProcessGtpuDataInd( NW_IN NwSdpHandleT hSdp, 
                    NW_IN NwU8T* udpData,
                    NW_IN NwU32T udpDataLen,
                    NW_IN NwU16T peerPort,
                    NW_IN NwU32T peerIp);

/*---------------------------------------------------------------------------
 * Process Request from Upper Layer
 *--------------------------------------------------------------------------*/

/**
 Process Request from ULP entity.

 @param[in] hSdp : SDP handle
 @param[in] pLogMgr : Pointer to Ulp Req.
 @return NW_SDP_OK on success.
 */

NwSdpRcT
nwSdpProcessUlpReq( NW_IN NwSdpHandleT hSdp,
                       NW_IN NwSdpUlpApiT *ulpReq);


/*---------------------------------------------------------------------------
 * Process Timer timeout Request from Timer Manager
 *--------------------------------------------------------------------------*/

/**
 Process Timer timeout Request from Timer Manager

 @param[in] pLogMgr : Pointer timeout arguments.
 @return NW_SDP_OK on success.
 */

NwSdpRcT
nwSdpProcessTimeout( NW_IN void* timeoutArg);

#ifdef __cplusplus
}
#endif

#endif  /* __NW_SDP_H__ */

/*--------------------------------------------------------------------------*
 *                      E N D     O F    F I L E                            *
 *--------------------------------------------------------------------------*/


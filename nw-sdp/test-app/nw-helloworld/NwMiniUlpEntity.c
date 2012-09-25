/*----------------------------------------------------------------------------*
 *                                                                            *
 *            M I N I M A L I S T I C     U L P     E N T I T Y               *
 *                                                                            *
 *                    Copyright (C) 2010 Amit Chawre.                         *
 *                                                                            *
 *----------------------------------------------------------------------------*/

/** 
 * @file NwMiniUlpEntity.c
 * @brief This file contains example of a minimalistic ULP entity.
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "NwEvt.h"
#include "NwSdp.h"
#include "NwMiniLogMgrEntity.h"
#include "NwMiniUlpEntity.h"

#ifndef NW_ASSERT
#define NW_ASSERT assert
#endif 

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------
 * Private Functions
 *--------------------------------------------------------------------------*/

#define MAX_UDP_PAYLOAD_LEN                     (4096)


/*---------------------------------------------------------------------------
 * Public Functions
 *--------------------------------------------------------------------------*/

NwSdpRcT
nwMiniUlpInit(NwMiniUlpEntityT* thiz, NwSdpHandleT hSdp)
{
  NwSdpRcT rc;
 thiz->hSdp = hSdp;

  return NW_SDP_OK;
}

NwSdpRcT
nwMiniUlpDestroy(NwMiniUlpEntityT* thiz)
{
  NW_ASSERT(thiz);
  memset(thiz, 0, sizeof(NwMiniUlpEntityT));
  return NW_SDP_OK;
}

NwSdpRcT
nwMiniUlpCreateConn(NwMiniUlpEntityT* thiz, char* localIpStr, NwU16T localport, char* peerIpStr)
{
  NwSdpRcT rc;
  int sd;
  struct sockaddr_in addr;
  NwSdpUlpApiT           ulpReq;

  strcpy(thiz->peerIpStr, peerIpStr);

  /*
   * Create local tunnel endpoint
   */

  ulpReq.apiType                              = NW_SDP_ULP_API_CREATE_FLOW;
  ulpReq.apiInfo.createFlowInfo.hUlpSession   = (NwSdpUlpSessionHandleT)thiz;

  ulpReq.apiInfo.createFlowInfo.egressEndPoint.ipv4Addr                 = inet_addr(peerIpStr);
  ulpReq.apiInfo.createFlowInfo.egressEndPoint.flowType                 = NW_FLOW_TYPE_GTPU;
  ulpReq.apiInfo.createFlowInfo.egressEndPoint.flowKey.greKey           = localport;

  ulpReq.apiInfo.createFlowInfo.ingressEndPoint.ipv4Addr                = inet_addr(localIpStr);
  ulpReq.apiInfo.createFlowInfo.ingressEndPoint.flowType                = NW_FLOW_TYPE_UDP;
  ulpReq.apiInfo.createFlowInfo.ingressEndPoint.flowKey.udp.port        = localport;
  ulpReq.apiInfo.createFlowInfo.ingressEndPoint.flowKey.udp.ipv4Addr    = inet_addr(localIpStr);

  rc = nwSdpProcessUlpReq(thiz->hSdp, &ulpReq);
  NW_ASSERT( rc == NW_SDP_OK );

  /*
   * Create local udp listening endpoint
   */

  return NW_SDP_OK;
}

NwSdpRcT
nwMiniUlpDestroyConn(NwMiniUlpEntityT* thiz)
{
  NwSdpRcT rc;
  NwSdpUlpApiT           ulpReq;
  /*---------------------------------------------------------------------------
   *  Send Destroy Session Request to GTPv1u Stack Instance
   *--------------------------------------------------------------------------*/

  ulpReq.apiType = NW_SDP_ULP_API_DESTROY_FLOW;
  ulpReq.apiInfo.destroyFlowInfo.hSdpSession = thiz->hGtpv1uConn;

  rc = nwSdpProcessUlpReq(thiz->hSdp, &ulpReq);
  NW_ASSERT( rc == NW_SDP_OK );

  thiz->hGtpv1uConn = 0;

  return NW_SDP_OK;
}


NwSdpRcT
nwMiniUlpTpduSend(NwMiniUlpEntityT* thiz, NwU8T* tpduBuf, NwU32T tpduLen , NwU16T fromPort)
{
  NwSdpRcT rc;
  NwSdpUlpApiT           ulpReq;

#if 0
  /*
   *  Send Message Request to GTPv1u Stack Instance
   */

  ulpReq.apiType                        = NW_GTPV1U_ULP_API_SEND_TPDU;
  ulpReq.apiInfo.sendtoInfo.teid        = fromPort;
  ulpReq.apiInfo.sendtoInfo.ipAddr      = inet_addr(thiz->peerIpStr);

  rc = nwSdpGpduMsgNew( thiz->hSdp,
      fromPort,
      NW_FALSE,
      thiz->seqNum++,
      tpduBuf,
      tpduLen,
      &(ulpReq.apiInfo.sendtoInfo.hMsg));

  NW_ASSERT( rc == NW_SDP_OK );

  rc = nwSdpProcessUlpReq(thiz->hSdp, &ulpReq);
  NW_ASSERT( rc == NW_SDP_OK );

  rc = nwSdpMsgDelete(thiz->hSdp, (ulpReq.apiInfo.sendtoInfo.hMsg));
  NW_ASSERT( rc == NW_SDP_OK );
#endif

  return NW_SDP_OK;
}

NwSdpRcT 
nwMiniUlpProcessStackReqCallback (NwSdpUlpHandleT hUlp, 
                       NwSdpUlpApiT *pUlpApi)
{
  NwMiniUlpEntityT* thiz;
  NW_ASSERT(pUlpApi != NULL);

  thiz = (NwMiniUlpEntityT*) hUlp;

  switch(pUlpApi->apiType)
  {
    default:
      NW_LOG(NW_LOG_LEVEL_WARN, "Received undefined UlpApi from gtpv1u stack!");
  }
  return NW_SDP_OK;
}

#ifdef __cplusplus
}
#endif


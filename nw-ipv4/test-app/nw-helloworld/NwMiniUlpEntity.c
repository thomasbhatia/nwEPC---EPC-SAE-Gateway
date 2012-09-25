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
#include "NwIpv4.h"
#include "NwIpv4Ie.h"
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

NwIpv4RcT
nwMiniUlpSendEchoRequestToPeer(NwMiniUlpEntityT* thiz)
{
  NwIpv4RcT rc;
  NwIpv4UlpApiT           ulpReq;

#if 0
  /*
   *  Send Message Request to Ipv4 Stack Instance
   */

  ulpReq.apiType = NW_IPv4_ULP_API_SEND_TPDU;

  ulpReq.apiInfo.initialReqInfo.hUlpTrxn        = (NwIpv4UlpTrxnHandleT)thiz;
  ulpReq.apiInfo.initialReqInfo.ipv4Addr            = 0x00;
  ulpReq.apiInfo.initialReqInfo.peerIp          = ntohl(inet_addr(thiz->peerIpStr));
  ulpReq.apiInfo.initialReqInfo.peerPort        = 2152;

  rc = nwIpv4MsgNew( thiz->hIpv4Stack,
      NW_TRUE,
      NW_GTP_ECHO_REQ,
      0,
      0,
      &(ulpReq.apiInfo.initialReqInfo.hMsg));

  NW_ASSERT( rc == NW_IPv4_OK );

  rc = nwIpv4MsgAddIeTV1((ulpReq.apiInfo.initialReqInfo.hMsg), NW_IPv4_IE_RECOVERY, 0, thiz->restartCounter);
  NW_ASSERT( rc == NW_IPv4_OK );

  rc = nwIpv4ProcessUlpReq(thiz->hIpv4Stack, &ulpReq);
  NW_ASSERT( rc == NW_IPv4_OK );
#endif

  return NW_IPv4_OK;
}

static
void NW_EVT_CALLBACK(nwMiniUlpDataIndicationCallbackData)
{
  NwMiniUlpEntityT* thiz = (NwMiniUlpEntityT*) arg;
  NwIpv4RcT         rc;
  NwU8T         udpBuf[MAX_UDP_PAYLOAD_LEN];
  NwS32T        bytesRead;
  NwU32T        peerLen;
  struct sockaddr_in peer;

  peerLen = sizeof(peer);

  bytesRead = recvfrom(thiz->hSocket, udpBuf, MAX_UDP_PAYLOAD_LEN , 0, (struct sockaddr *) &peer,(socklen_t*) &peerLen);
  if(bytesRead)
  {
    NW_LOG(NW_LOG_LEVEL_DEBG, "Received UDP message of length %u from %X:%u", bytesRead, ntohl(peer.sin_addr.s_addr), ntohs(peer.sin_port));
    rc = nwMiniUlpTpduSend(thiz, udpBuf, bytesRead, thiz->localPort[thiz->hSocket]);
  }
  else
  {
    NW_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
  } 
}

/*---------------------------------------------------------------------------
 * Public Functions
 *--------------------------------------------------------------------------*/

NwIpv4RcT
nwMiniUlpInit(NwMiniUlpEntityT* thiz, NwIpv4StackHandleT hIpv4Stack)
{
  NwIpv4RcT rc;
 thiz->hIpv4Stack = hIpv4Stack;

  return NW_IPv4_OK;
}

NwIpv4RcT
nwMiniUlpDestroy(NwMiniUlpEntityT* thiz)
{
  NW_ASSERT(thiz);
  memset(thiz, 0, sizeof(NwMiniUlpEntityT));
  return NW_IPv4_OK;
}

NwIpv4RcT
nwMiniUlpCreateConn(NwMiniUlpEntityT* thiz, char* localIpStr, NwU16T localport, char* peerIpStr)
{
  NwIpv4RcT rc;
  int sd;
  struct sockaddr_in addr;
  NwIpv4UlpApiT           ulpReq;

  strcpy(thiz->peerIpStr, peerIpStr);

  /*
   * Create local tunnel endpoint
   */

  NW_LOG(NW_LOG_LEVEL_NOTI, "Creating tunnel endpoint with ipv4Addr %d", localport); 
  ulpReq.apiType                                        = NW_IPv4_ULP_API_CREATE_TUNNEL_ENDPOINT;
  ulpReq.apiInfo.createTunnelEndPointInfo.ipv4Addr          = localport;
  ulpReq.apiInfo.createTunnelEndPointInfo.hUlpSession   = (NwIpv4UlpSessionHandleT)thiz;

  rc = nwIpv4ProcessUlpReq(thiz->hIpv4Stack, &ulpReq);
  NW_ASSERT( rc == NW_IPv4_OK );

  thiz->hIpv4Conn = ulpReq.apiInfo.createTunnelEndPointInfo.hStackSession;

  /*
   * Create local udp listening endpoint
   */

  sd = socket(AF_INET, SOCK_DGRAM, 0);

  if (sd < 0)
  {
    NW_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
    NW_ASSERT(0);
  }

  addr.sin_family       = AF_INET;
  addr.sin_port         = htons(localport);
  addr.sin_addr.s_addr  = inet_addr(localIpStr);
  memset(addr.sin_zero, '\0', sizeof (addr.sin_zero));

  if(bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    NW_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
    NW_ASSERT(0);
  }

  NW_EVENT_ADD((thiz->ev[sd]), sd, nwMiniUlpDataIndicationCallbackData, thiz, NW_EVT_READ | NW_EVT_PERSIST);

  thiz->localPort[sd] = localport;

  /*
   * Create local udp for sendign data
   */

  sd = socket(AF_INET, SOCK_DGRAM, 0);

  if (sd < 0)
  {
    NW_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
    NW_ASSERT(0);
  }


  thiz->hSocket = sd;
  return NW_IPv4_OK;
}

NwIpv4RcT
nwMiniUlpDestroyConn(NwMiniUlpEntityT* thiz)
{
  NwIpv4RcT rc;
  NwIpv4UlpApiT           ulpReq;
  /*---------------------------------------------------------------------------
   *  Send Destroy Session Request to IPv4 Stack Instance
   *--------------------------------------------------------------------------*/

  ulpReq.apiType = NW_IPv4_ULP_API_DESTROY_TUNNEL_ENDPOINT;
  ulpReq.apiInfo.destroyTunnelEndPointInfo.hStackSessionHandle = thiz->hIpv4Conn;

  rc = nwIpv4ProcessUlpReq(thiz->hIpv4Stack, &ulpReq);
  NW_ASSERT( rc == NW_IPv4_OK );

  thiz->hIpv4Conn = 0;

  return NW_IPv4_OK;
}


NwIpv4RcT
nwMiniUlpTpduSend(NwMiniUlpEntityT* thiz, NwU8T* tpduBuf, NwU32T tpduLen , NwU16T fromPort)
{
  NwIpv4RcT rc;
  NwIpv4UlpApiT           ulpReq;

#if 0
  /*
   *  Send Message Request to IPv4 Stack Instance
   */

  ulpReq.apiType                        = NW_IPv4_ULP_API_SEND_TPDU;
  ulpReq.apiInfo.sendtoInfo.ipv4Addr        = fromPort;
  ulpReq.apiInfo.sendtoInfo.ipAddr      = inet_addr(thiz->peerIpStr);

  rc = nwIpv4GpduMsgNew( thiz->hIpv4Stack,
      NW_FALSE,
      NW_TRUE,
      NW_FALSE,
      fromPort,
      thiz->seqNum++,
      tpduBuf,
      tpduLen,
      &(ulpReq.apiInfo.sendtoInfo.hMsg));

  NW_ASSERT( rc == NW_IPv4_OK );

  rc = nwIpv4ProcessUlpReq(thiz->hIpv4Stack, &ulpReq);
  NW_ASSERT( rc == NW_IPv4_OK );


  rc = nwIpv4MsgDelete(thiz->hIpv4Stack, (ulpReq.apiInfo.sendtoInfo.hMsg));
  NW_ASSERT( rc == NW_IPv4_OK );
#endif

  return NW_IPv4_OK;
}

NwIpv4RcT 
nwMiniUlpProcessStackReqCallback (NwIpv4UlpHandleT hUlp, 
                       NwIpv4UlpApiT *pUlpApi)
{
  NwMiniUlpEntityT* thiz;
  NW_ASSERT(pUlpApi != NULL);

  thiz = (NwMiniUlpEntityT*) hUlp;

  switch(pUlpApi->apiType)
  {
    case NW_IPv4_ULP_API_RECV_TPDU:
      {
        struct sockaddr_in peerAddr;
        NwS32T bytesSent;
        NwU8T dataBuf[4096];
        NwU32T dataSize;
        NwU32T peerIpAddr = (inet_addr(thiz->peerIpStr));

#if 0
        NW_ASSERT( NW_IPv4_OK == nwIpv4MsgGetTpdu(pUlpApi->apiInfo.recvMsgInfo.hMsg, dataBuf, &dataSize) );

        NW_LOG(NW_LOG_LEVEL_DEBG, "Received TPDU from IPv4 stack!");

        peerAddr.sin_family       = AF_INET;
        peerAddr.sin_port         = htons(pUlpApi->apiInfo.recvMsgInfo.ipv4Addr);
        peerAddr.sin_addr.s_addr  = (peerIpAddr);
        memset(peerAddr.sin_zero, '\0', sizeof (peerAddr.sin_zero));

        bytesSent = sendto (thiz->hSocket, dataBuf, dataSize, 0, (struct sockaddr *) &peerAddr, sizeof(peerAddr));

        if(bytesSent < 0)
        {
          NW_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
          NW_ASSERT(0);
        }
        else
        {
          NW_LOG(NW_LOG_LEVEL_DEBG, "Sent %u bytes to peer %u.%u.%u.%u", dataSize,
              (peerIpAddr & 0x000000ff),
              (peerIpAddr & 0x0000ff00) >> 8,
              (peerIpAddr & 0x00ff0000) >> 16,
              (peerIpAddr & 0xff000000) >> 24);
        }
#endif

        NW_ASSERT(nwIpv4MsgDelete(thiz->hIpv4Stack, (pUlpApi->apiInfo.recvMsgInfo.hMsg)) == NW_IPv4_OK);

      }
      break;
    default:
      NW_LOG(NW_LOG_LEVEL_WARN, "Received undefined UlpApi from Ipv4 stack!");
  }
  return NW_IPv4_OK;
}

#ifdef __cplusplus
}
#endif


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
#include "NwGre.h"
#include "NwGreIe.h"
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

NwRcT
nwMiniUlpSendEchoRequestToPeer(NwMiniUlpEntityT* thiz)
{
  NwRcT rc;
  NwGreUlpApiT           ulpReq;

  /*
   *  Send Message Request to Gre Stack Instance
   */

  ulpReq.apiType = NW_GRE_ULP_API_INITIAL_REQ;

  ulpReq.apiInfo.initialReqInfo.hUlpTrxn        = (NwGreUlpTrxnHandleT)thiz;
  ulpReq.apiInfo.initialReqInfo.greKey          = 0x00;
  ulpReq.apiInfo.initialReqInfo.peerIp          = ntohl(inet_addr(thiz->peerIpStr));
  ulpReq.apiInfo.initialReqInfo.peerPort        = 2152;

  rc = nwGreMsgNew( thiz->hGreStack,
      NW_TRUE,
      NW_GTP_ECHO_REQ,
      0,
      0,
      &(ulpReq.apiInfo.initialReqInfo.hMsg));

  NW_ASSERT( NW_OK == rc );

  rc = nwGreMsgAddIeTV1((ulpReq.apiInfo.initialReqInfo.hMsg), NW_GRE_IE_RECOVERY, 0, thiz->restartCounter);
  NW_ASSERT( NW_OK == rc );

  rc = nwGreProcessUlpReq(thiz->hGreStack, &ulpReq);
  NW_ASSERT( NW_OK == rc );

  return NW_OK;
}

static
void NW_TMR_CALLBACK(nwMiniUlpDataIndicationCallbackData)
{
  NwMiniUlpEntityT* thiz = (NwMiniUlpEntityT*) arg;
  NwRcT         rc;
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

NwRcT
nwMiniUlpInit(NwMiniUlpEntityT* thiz, NwGreStackHandleT hGreStack)
{
  NwRcT rc;
 thiz->hGreStack = hGreStack;

  return NW_OK;
}

NwRcT
nwMiniUlpDestroy(NwMiniUlpEntityT* thiz)
{
  NW_ASSERT(thiz);
  memset(thiz, 0, sizeof(NwMiniUlpEntityT));
  return NW_OK;
}

NwRcT
nwMiniUlpCreateConn(NwMiniUlpEntityT* thiz, char* localIpStr, NwU16T localport, char* peerIpStr)
{
  NwRcT rc;
  int sd;
  struct sockaddr_in addr;
  NwGreUlpApiT           ulpReq;

  strcpy(thiz->peerIpStr, peerIpStr);

  /*
   * Create local tunnel endpoint
   */

  NW_LOG(NW_LOG_LEVEL_NOTI, "Creating tunnel endpoint with GRE key %d", localport); 
  ulpReq.apiType                                        = NW_GRE_ULP_API_CREATE_TUNNEL_ENDPOINT;
  ulpReq.apiInfo.createTunnelEndPointInfo.greKey        = localport;
  ulpReq.apiInfo.createTunnelEndPointInfo.hUlpSession   = (NwGreUlpSessionHandleT)thiz;

  rc = nwGreProcessUlpReq(thiz->hGreStack, &ulpReq);
  NW_ASSERT( NW_OK == rc );

  thiz->hGreConn = ulpReq.apiInfo.createTunnelEndPointInfo.hStackSession;

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
  return NW_OK;
}

NwRcT
nwMiniUlpDestroyConn(NwMiniUlpEntityT* thiz)
{
  NwRcT rc;
  NwGreUlpApiT           ulpReq;
  /*---------------------------------------------------------------------------
   *  Send Destroy Session Request to GRE Stack Instance
   *--------------------------------------------------------------------------*/

  ulpReq.apiType = NW_GRE_ULP_API_DESTROY_TUNNEL_ENDPOINT;
  ulpReq.apiInfo.destroyTunnelEndPointInfo.hStackSessionHandle = thiz->hGreConn;

  rc = nwGreProcessUlpReq(thiz->hGreStack, &ulpReq);
  NW_ASSERT( NW_OK == rc );

  thiz->hGreConn = 0;

  return NW_OK;
}


NwRcT
nwMiniUlpTpduSend(NwMiniUlpEntityT* thiz, NwU8T* tpduBuf, NwU32T tpduLen , NwU16T fromPort)
{
  NwRcT rc;
  NwGreUlpApiT           ulpReq;

  /*
   *  Send Message Request to GRE Stack Instance
   */

  ulpReq.apiType                        = NW_GRE_ULP_API_SEND_TPDU;
  ulpReq.apiInfo.sendtoInfo.greKey      = fromPort;
  ulpReq.apiInfo.sendtoInfo.ipAddr      = inet_addr(thiz->peerIpStr);

  rc = nwGreGpduMsgNew( thiz->hGreStack,
      NW_FALSE,
      NW_TRUE,
      NW_FALSE,
      fromPort,
      thiz->seqNum++,
      tpduBuf,
      tpduLen,
      &(ulpReq.apiInfo.sendtoInfo.hMsg));

  NW_ASSERT( NW_OK == rc );

  rc = nwGreProcessUlpReq(thiz->hGreStack, &ulpReq);
  NW_ASSERT( NW_OK == rc );

  rc = nwGreMsgDelete(thiz->hGreStack, (ulpReq.apiInfo.sendtoInfo.hMsg));
  NW_ASSERT( NW_OK == rc );

  return NW_OK;
}

NwRcT 
nwMiniUlpProcessStackReqCallback (NwGreUlpHandleT hUlp, 
                       NwGreUlpApiT *pUlpApi)
{
  NwMiniUlpEntityT* thiz;
  NW_ASSERT(pUlpApi != NULL);

  thiz = (NwMiniUlpEntityT*) hUlp;

  switch(pUlpApi->apiType)
  {
    case NW_GRE_ULP_API_RECV_TPDU:
      {
        struct sockaddr_in peerAddr;
        NwS32T bytesSent;
        NwU8T dataBuf[4096];
        NwU32T dataSize;
        NwU32T peerIpAddr = (inet_addr(thiz->peerIpStr));

        NW_ASSERT( NW_OK == nwGreMsgGetTpdu(pUlpApi->apiInfo.recvMsgInfo.hMsg, dataBuf, &dataSize) );

        NW_LOG(NW_LOG_LEVEL_DEBG, "Received TPDU from GRE stack %u!", pUlpApi->apiInfo.recvMsgInfo.greKey);

        peerAddr.sin_family       = AF_INET;
        peerAddr.sin_port         = htons(pUlpApi->apiInfo.recvMsgInfo.greKey);
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

        NW_ASSERT(nwGreMsgDelete(thiz->hGreStack, (pUlpApi->apiInfo.recvMsgInfo.hMsg)) == NW_OK);

      }
      break;
    default:
      NW_LOG(NW_LOG_LEVEL_WARN, "Received undefined UlpApi from gre stack!");
  }
  return NW_OK;
}

#ifdef __cplusplus
}
#endif


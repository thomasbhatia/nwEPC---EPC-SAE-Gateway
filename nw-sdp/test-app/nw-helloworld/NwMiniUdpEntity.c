/*----------------------------------------------------------------------------*
 *                                                                            *
 *            M I N I M A L I S T I C     U D P     E N T I T Y               *
 *                                                                            *
 *                    Copyright (C) 2010 Amit Chawre.                         *
 *                                                                            *
 *----------------------------------------------------------------------------*/


/** 
 * @file NwMiniUdpEntity.c
 * @brief This file contains example of a minimalistic ULP entity.
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "NwEvt.h"
#include "NwSdp.h"
#include "NwMiniLogMgrEntity.h"
#include "NwMiniUdpEntity.h"

#ifndef NW_ASSERT
#define NW_ASSERT assert
#endif 

#define MAX_UDP_PAYLOAD_LEN             (4096)

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------
 * Private functions
 *--------------------------------------------------------------------------*/

typedef struct 
{
  int           hSocket;
  NwEventT      ev;
  NwU32T        hSdp;
  NwU32T        hSdpService;
  NwU32T        ipv4Addr;
} NwGreInterfaceT;

static
void NW_EVT_CALLBACK(nwSdpGreDataIndicationCallbackData)
{
  NwSdpRcT         rc;
#define MAX_GRE_PAYLOAD_LEN             (4096)
  NwU8T         greBuf[MAX_GRE_PAYLOAD_LEN];
  NwS32T        bytesRead;
  NwU32T        peerLen;
  struct sockaddr_in peer;
  NwGreInterfaceT* thiz = (NwGreInterfaceT*) arg;

  peerLen = sizeof(peer);

  bytesRead = recvfrom(thiz->hSocket, greBuf, MAX_GRE_PAYLOAD_LEN , 0, (struct sockaddr *) &peer,(socklen_t*) &peerLen);
  if(bytesRead)
  {
    NW_LOG(NW_LOG_LEVEL_DEBG, "Received UDP message of length %u from %X:%u", bytesRead, ntohl(peer.sin_addr.s_addr), ntohs(peer.sin_port));
    rc = nwSdpProcessGreDataInd(thiz->hSdp, greBuf, bytesRead, peer.sin_port, peer.sin_addr.s_addr);
  }
  else
  {
    NW_LOG( NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
  }
}

static
void NW_EVT_CALLBACK(nwUdpDataIndicationCallbackData)
{
  NwSdpRcT         rc;
  NwU8T         udpBuf[MAX_UDP_PAYLOAD_LEN];
  NwS32T        bytesRead;
  NwU32T        peerLen;
  struct sockaddr_in peer;
  NwMiniUdpEntityT* thiz = (NwMiniUdpEntityT*) arg;

  peerLen = sizeof(peer);

  bytesRead = recvfrom(thiz->hSocket, udpBuf, MAX_UDP_PAYLOAD_LEN , 0, (struct sockaddr *) &peer,(socklen_t*) &peerLen);
  if(bytesRead)
  {
    NW_LOG(NW_LOG_LEVEL_DEBG, "Received UDP message of length %u from %X:%u", bytesRead, ntohl(peer.sin_addr.s_addr), ntohs(peer.sin_port));
    rc = nwSdpProcessUdpDataInd(thiz->hSdp, udpBuf, bytesRead, peer.sin_port, peer.sin_addr.s_addr);
  }
  else
  {
    NW_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
  }
}


/*---------------------------------------------------------------------------
 * Public functions
 *--------------------------------------------------------------------------*/

NwSdpRcT nwMiniUdpInit(NwMiniUdpEntityT* thiz, NwSdpHandleT hSdp, NwU8T* ipAddr)
{
  int sd;
  struct sockaddr_in addr;

  sd = socket(AF_INET, SOCK_DGRAM, 0);

  if (sd < 0)
  {
    NW_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
    NW_ASSERT(0);
  }

  addr.sin_family       = AF_INET;
  addr.sin_port         = htons(2152);
  addr.sin_addr.s_addr  = inet_addr(ipAddr);
  memset(addr.sin_zero, '\0', sizeof (addr.sin_zero));

  if(bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    NW_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
    NW_ASSERT(0);
  }

  NW_EVENT_ADD((thiz->ev), sd, nwUdpDataIndicationCallbackData, thiz, (EV_READ|EV_PERSIST));

  thiz->hSocket = sd;
  thiz->hSdp = hSdp;

  return NW_SDP_OK;
}

NwSdpRcT nwMiniUdpDestroy(NwMiniUdpEntityT* thiz)
{
  close(thiz->hSocket);
}

NwSdpRcT
nwMiniUdpCreateGreInterface(NwMiniUdpEntityT* thiz, char* localIpStr, NwU32T* phGreInterface)
{
  NwGreInterfaceT* pGreIf;

  pGreIf = (NwGreInterfaceT*) malloc (sizeof(NwGreInterfaceT));

  /*
   * Create a GRE socket
   */

#ifndef IPPROTO_GRE
#define IPPROTO_GRE     (47)
#endif
  pGreIf->hSocket = socket(AF_INET, SOCK_RAW, IPPROTO_GRE);

  if (pGreIf->hSocket < 0)
  {
    NW_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
    NW_ASSERT(0);
  }
  struct sockaddr_in addr;
  addr.sin_family       = AF_INET;
  addr.sin_addr.s_addr  = inet_addr(localIpStr);
  memset(addr.sin_zero, '\0', sizeof (addr.sin_zero));

  if(bind(pGreIf->hSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    NW_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
    NW_ASSERT(0);
  }

  pGreIf->ipv4Addr      = addr.sin_addr.s_addr;

  NW_EVENT_ADD((thiz->ev), pGreIf->hSocket, nwSdpGreDataIndicationCallbackData, thiz, (EV_READ|EV_PERSIST));
}

NwSdpRcT
nwMiniUdpGreInterfaceSetSdpService(NwMiniUdpEntityT* thiz, NwU32T hGreInterface, NwU32T hSdp, NwU32T hSdpService)
{
  NwGreInterfaceT* pGreIf = (NwGreInterfaceT*) hGreInterface;
  pGreIf->hSdp          = hSdp;
  pGreIf->ipv4Addr      = hSdpService;

  return NW_SDP_OK;
}

NwSdpRcT
nwMiniUdpDestroyGreInterface(NwMiniUdpEntityT* thiz, NwU32T hGreInterface)
{
  NwGreInterfaceT* pGreIf = (NwGreInterfaceT*) hGreInterface;
  event_del(&(pGreIf->ev));
  close(pGreIf->hSocket);
  return NW_SDP_OK;
}

NwSdpRcT nwMiniUdpGreDataReq(NwU32T hGreInterface,
    NwU8T* dataBuf,
    NwU32T dataSize,
    NwU32T peerIpAddr,
    NwU32T peerPort)
{
  struct sockaddr_in peerAddr;
  NwS32T bytesSent;
  NwGreInterfaceT* thiz = (NwGreInterfaceT*) hGreInterface;

  peerAddr.sin_family       = AF_INET;
  peerAddr.sin_port         = htons(peerPort);
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
    NW_LOG(NW_LOG_LEVEL_DEBG, "Sent %u bytes on handle 0x%x to peer %u.%u.%u.%u", dataSize, hGreInterface,
        (peerIpAddr & 0x000000ff),
        (peerIpAddr & 0x0000ff00) >> 8,
        (peerIpAddr & 0x00ff0000) >> 16,
        (peerIpAddr & 0xff000000) >> 24);

  }
  return NW_SDP_OK;
}

NwSdpRcT nwMiniUdpDataReq(NwSdpUdpHandleT udpHandle,
    NwU8T* dataBuf,
    NwU32T dataSize,
    NwU32T peerIpAddr,
    NwU32T peerPort)
{
  struct sockaddr_in peerAddr;
  NwS32T bytesSent;
  NwMiniUdpEntityT* thiz = (NwMiniUdpEntityT*) udpHandle;

  peerAddr.sin_family       = AF_INET;
  peerAddr.sin_port         = htons(peerPort);
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
    NW_LOG(NW_LOG_LEVEL_DEBG, "Sent %u bytes on handle 0x%x to peer %u.%u.%u.%u", dataSize, udpHandle,
        (peerIpAddr & 0x000000ff),
        (peerIpAddr & 0x0000ff00) >> 8,
        (peerIpAddr & 0x00ff0000) >> 16,
        (peerIpAddr & 0xff000000) >> 24);

  }
  return NW_SDP_OK;
}



#ifdef __cplusplus
}
#endif


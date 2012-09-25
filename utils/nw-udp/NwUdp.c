

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "NwEvt.h"
#include "NwUtils.h"
#include "NwLog.h"
#include "NwUdp.h"
#include "NwUdpLog.h"

/*---------------------------------------------------------------------------
 *                          U D P     E N T I T Y 
 *--------------------------------------------------------------------------*/

static void NW_EVT_CALLBACK(nwUdpDataIndicationCallbackData)
{
  NwRcT         rc;
  NwU8T         udpBuf[MAX_UDP_PAYLOAD_LEN];
  NwU32T        bytesRead;
  NwU32T        peerLen;
  struct sockaddr_in peer;
  NwUdpT* thiz = (NwUdpT*) arg;

  peerLen = sizeof(peer);

  bytesRead = recvfrom(thiz->hSocket, udpBuf, MAX_UDP_PAYLOAD_LEN , 0, (struct sockaddr *) &peer,(socklen_t*) &peerLen);
  if(bytesRead)
  {
    NW_UDP_LOG(NW_LOG_LEVEL_DEBG, "Received UDP message of length %u from %X:%u", bytesRead, ntohl(peer.sin_addr.s_addr), ntohs(peer.sin_port));
    nwLogHexDump(udpBuf, bytesRead);

    rc = nwGtpv2cProcessUdpReq(thiz->hGtpcStack, udpBuf, bytesRead, ntohs(peer.sin_port), (peer.sin_addr.s_addr));
  }
  else
  {
    NW_UDP_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
  }
}

NwRcT nwUdpDataReq(NwGtpv2cUdpHandleT udpHandle,
    NwU8T* dataBuf,
    NwU32T dataSize,
    NwU32T peerIpAddr,
    NwU32T peerPort)
{
  struct sockaddr_in peerAddr;
  NwS32T bytesSent;
  NwUdpT* thiz = (NwUdpT*) udpHandle;

  NW_UDP_LOG(NW_LOG_LEVEL_DEBG, "Sending buf of size %u for on handle %x to peer %u.%u.%u.%u:%u", dataSize, udpHandle,
      (peerIpAddr & 0x000000ff),
      (peerIpAddr & 0x0000ff00) >> 8,
      (peerIpAddr & 0x00ff0000) >> 16,
      (peerIpAddr & 0xff000000) >> 24,
      peerPort);

  peerAddr.sin_family       = AF_INET;
  peerAddr.sin_port         = htons(peerPort);
  peerAddr.sin_addr.s_addr  = (peerIpAddr);
  memset(peerAddr.sin_zero, '\0', sizeof (peerAddr.sin_zero));
  
  nwLogHexDump(dataBuf, dataSize);

  bytesSent = sendto (thiz->hSocket, dataBuf, dataSize, 0, (struct sockaddr *) &peerAddr, sizeof(peerAddr));

  if(bytesSent < 0)
  {
    NW_UDP_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
    NW_ASSERT(0);
  }
  return NW_OK;
}

NwRcT nwUdpInit(NwUdpT* thiz, NwU32T ipAddr, NwGtpv2cStackHandleT hGtpcStack)
{
  int sd;
  struct sockaddr_in addr;

  sd = socket(AF_INET, SOCK_DGRAM, 0);

  if (sd < 0)
  {
    NW_UDP_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
    NW_ASSERT(0);
  }

  addr.sin_family       = AF_INET;
  addr.sin_port         = htons(NW_GTPC_UDP_PORT);
  addr.sin_addr.s_addr  = (ipAddr);
  memset(addr.sin_zero, '\0', sizeof (addr.sin_zero));

  if(bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    NW_UDP_LOG(NW_LOG_LEVEL_ERRO, "Bind error for %x:%u - %s", ipAddr, NW_GTPC_UDP_PORT, strerror(errno));
    NW_ASSERT(0);
  }

  NW_EVENT_ADD((thiz->ev), sd, nwUdpDataIndicationCallbackData, thiz, NW_EVT_READ | NW_EVT_PERSIST);

  thiz->hSocket         = sd;
  thiz->hGtpcStack      = hGtpcStack;
  thiz->ipAddr          = ipAddr;

  return NW_OK;
}



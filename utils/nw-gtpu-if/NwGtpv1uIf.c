

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
#include "NwGtpv1u.h"
#include "NwGtpv1uIf.h"
#include "NwGtpv1uIfLog.h"

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------
 *                          U D P     E N T I T Y 
 *--------------------------------------------------------------------------*/

NwRcT nwGtpv1uIfInitialize(NwGtpv1uIfT* thiz, NwU32T ipAddr, NwSdpHandleT hSdp)
{
  int sd;
  struct sockaddr_in addr;

  sd = socket(AF_INET, SOCK_DGRAM, 0);

  if (sd < 0)
  {
    NW_GTPV1U_IF_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
    NW_ASSERT(0);
  }

  addr.sin_family       = AF_INET;
  addr.sin_port         = htons(NW_GTPU_UDP_PORT);
  addr.sin_addr.s_addr  = htonl(ipAddr);
  memset(addr.sin_zero, '\0', sizeof (addr.sin_zero));

  if(bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    NW_GTPV1U_IF_LOG(NW_LOG_LEVEL_ERRO, "Bind error for %x:%u - %s", ipAddr, NW_GTPU_UDP_PORT, strerror(errno));
    NW_ASSERT(0);
  }

  thiz->hSocket         = sd;
  thiz->hSdp            = hSdp;
  thiz->ipAddr          = ipAddr;

  return NW_OK;
}

NwRcT nwGtpv1uIfGetSelectionObject(NwGtpv1uIfT* thiz, NwU32T *pSelObj)
{
  *pSelObj = thiz->hSocket;
  return NW_OK;
}

void NW_EVT_CALLBACK(nwGtpv1uIfDataIndicationCallback)
{
  NwRcT         rc;
  NwU8T         udpBuf[MAX_GTPU_PAYLOAD_LEN];
  NwU32T        bytesRead;
  NwU32T        peerLen;
  struct sockaddr_in peer;
  NwGtpv1uIfT* thiz = (NwGtpv1uIfT*) arg;

  peerLen = sizeof(peer);

  bytesRead = recvfrom(thiz->hSocket, udpBuf, MAX_GTPU_PAYLOAD_LEN , 0, (struct sockaddr *) &peer,(socklen_t*) &peerLen);
  if(bytesRead)
  {
    NW_GTPV1U_IF_LOG(NW_LOG_LEVEL_DEBG, "Received GTPU message of length %u from "NW_IPV4_ADDR":%u", bytesRead, NW_IPV4_ADDR_FORMAT((peer.sin_addr.s_addr)), ntohs(peer.sin_port));
    nwLogHexDump(udpBuf, bytesRead);

    rc = nwSdpProcessGtpuDataInd(thiz->hSdp, udpBuf, bytesRead, ntohs(peer.sin_port), ntohl(peer.sin_addr.s_addr));
  }
  else
  {
    NW_GTPV1U_IF_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
  }
}

NwRcT nwGtpv1uIfDataReq(NwGtpv1uUdpHandleT udpHandle,
    NwU8T* dataBuf,
    NwU32T dataSize,
    NwU32T peerIpAddr,
    NwU32T peerPort)
{
  struct sockaddr_in peerAddr;
  NwS32T bytesSent;
  NwGtpv1uIfT* thiz = (NwGtpv1uIfT*) udpHandle;

  NW_GTPV1U_IF_LOG(NW_LOG_LEVEL_DEBG, "Sending buf of size %u for on handle %x to peer "NW_IPV4_ADDR, dataSize, udpHandle,
      NW_IPV4_ADDR_FORMAT(ntohl(peerIpAddr)));

  peerAddr.sin_family       = AF_INET;
  peerAddr.sin_port         = htons(peerPort);
  peerAddr.sin_addr.s_addr  = htonl(peerIpAddr);
  memset(peerAddr.sin_zero, '\0', sizeof (peerAddr.sin_zero));
  
  nwLogHexDump(dataBuf, dataSize);

  bytesSent = sendto (thiz->hSocket, dataBuf, dataSize, 0, (struct sockaddr *) &peerAddr, sizeof(peerAddr));

  if(bytesSent < 0)
  {
    NW_GTPV1U_IF_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
    NW_ASSERT(0);
  }
  return NW_OK;
}

#ifdef __cplusplus
}
#endif



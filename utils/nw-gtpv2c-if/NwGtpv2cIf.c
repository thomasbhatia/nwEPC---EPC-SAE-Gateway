

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
#include "NwGtpv2cIf.h"
#include "NwGtpv2cIfLog.h"

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------
 *                          U D P     E N T I T Y 
 *--------------------------------------------------------------------------*/


NwRcT nwGtpv2cIfInitialize(NwGtpv2cIfT* thiz, NwU32T ipAddr, NwGtpv2cStackHandleT hGtpcStack)
{
  int sd;
  struct sockaddr_in addr;

  sd = socket(AF_INET, SOCK_DGRAM, 0);

  if (sd < 0)
  {
    NW_GTPV2C_IF_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
    NW_ASSERT(0);
  }

  addr.sin_family       = AF_INET;
  addr.sin_port         = htons(NW_GTPC_UDP_PORT);
  addr.sin_addr.s_addr  = htonl(ipAddr);
  memset(addr.sin_zero, '\0', sizeof (addr.sin_zero));

  if(bind(sd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    NW_GTPV2C_IF_LOG(NW_LOG_LEVEL_ERRO, "Bind error for %x:%u - %s", ipAddr, NW_GTPC_UDP_PORT, strerror(errno));
    NW_ASSERT(0);
  }

  thiz->hSocket         = sd;
  thiz->hGtpcStack      = hGtpcStack;
  thiz->ipAddr          = ipAddr;

  return NW_OK;
}

NwRcT nwGtpv2cIfGetSelectionObject(NwGtpv2cIfT* thiz, NwU32T *pSelObj)
{
  *pSelObj = thiz->hSocket;
  return NW_OK;
}

void NW_EVT_CALLBACK(nwGtpv2cIfDataIndicationCallback)
{
  NwRcT         rc;
  NwU8T         udpBuf[MAX_GTPV2C_PAYLOAD_LEN];
  NwU32T        bytesRead;
  NwU32T        peerLen;
  struct sockaddr_in peer;
  NwGtpv2cIfT* thiz = (NwGtpv2cIfT*) arg;

  peerLen = sizeof(peer);

  bytesRead = recvfrom(thiz->hSocket, udpBuf, MAX_GTPV2C_PAYLOAD_LEN , 0, (struct sockaddr *) &peer,(socklen_t*) &peerLen);
  if(bytesRead)
  {
    NW_GTPV2C_IF_LOG(NW_LOG_LEVEL_DEBG, "Received GTPCv2 message of length %u from %X:%u", bytesRead, ntohl(peer.sin_addr.s_addr), ntohs(peer.sin_port));
    nwLogHexDump(udpBuf, bytesRead);

    rc = nwGtpv2cProcessUdpReq(thiz->hGtpcStack, udpBuf, bytesRead, ntohs(peer.sin_port), (peer.sin_addr.s_addr));
  }
  else
  {
    NW_GTPV2C_IF_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
  }
}

NwRcT nwGtpv2cIfDataReq(NwGtpv2cUdpHandleT udpHandle,
    NwU8T* dataBuf,
    NwU32T dataSize,
    NwU32T peerIpAddr,
    NwU32T peerPort)
{
  struct sockaddr_in peerAddr;
  NwS32T bytesSent;
  NwGtpv2cIfT* thiz = (NwGtpv2cIfT*) udpHandle;

  NW_GTPV2C_IF_LOG(NW_LOG_LEVEL_DEBG, "Sending buf of size %u for on handle %x to peer "NW_IPV4_ADDR, dataSize, udpHandle,
      NW_IPV4_ADDR_FORMAT(peerIpAddr));

  peerAddr.sin_family       = AF_INET;
  peerAddr.sin_port         = htons(peerPort);
  peerAddr.sin_addr.s_addr  = (peerIpAddr);
  memset(peerAddr.sin_zero, '\0', sizeof (peerAddr.sin_zero));
  
  nwLogHexDump(dataBuf, dataSize);

  bytesSent = sendto (thiz->hSocket, dataBuf, dataSize, 0, (struct sockaddr *) &peerAddr, sizeof(peerAddr));

  if(bytesSent < 0)
  {
    NW_GTPV2C_IF_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
    NW_ASSERT(0);
  }
  return NW_OK;
}
#ifdef __cplusplus
}
#endif



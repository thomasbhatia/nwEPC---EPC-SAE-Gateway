/*----------------------------------------------------------------------------*
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

/** 
 * @file NwIpv4If.c
 * @brief This files defines IP interface entity.
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/sockios.h>

#include "NwEvt.h"
#include "NwUtils.h"
#include "NwLog.h"
#include "NwIpv4If.h"
#include "NwIpv4IfLog.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NwArpPacket
{
  NwU8T  dstMac[6];
  NwU8T  srcMac[6];
  NwU16T protocol;
  NwU16T hwType;
  NwU16T protoType;
  NwU8T  hwAddrLen;
  NwU8T  protoAddrLen;
  NwU16T opCode;
  NwU8T  senderMac[6];
  NwU8T  senderIpAddr[4];
  NwU8T  targetMac[6];
  NwU8T  targetIpAddr[4];
} NwArpPacketT;

/*---------------------------------------------------------------------------
 *                          I P     E N T I T Y 
 *--------------------------------------------------------------------------*/

NwRcT nwIpv4IfInitialize(NwIpv4IfT* thiz, NwU8T* device, NwSdpHandleT hSdp, NwU8T *pHwAddr)
{
  int sd, send_sd;
  struct sockaddr_ll sll;
  struct ifreq ifr;

  /*
   * Create Socket for listening IP packets
   */
  sd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));

  if (sd < 0)
  {
    NW_IP_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
    NW_ASSERT(0);
  }

  bzero(&sll, sizeof(sll));
  bzero(&ifr, sizeof(ifr));

  /* First Get the Interface Index  */

  strncpy((char *)ifr.ifr_name, (const char*)device, IFNAMSIZ);

  if((ioctl(sd, SIOCGIFHWADDR, &ifr)) == -1)
  {     
    printf("Error getting Interface hw address!\n");
    exit(-1);
  }
  else
  {
#if 0
    printf("HW address of interface is: %02x:%02x:%02x:%02x:%02x:%02x\n", 
        (unsigned char)ifr.ifr_ifru.ifru_hwaddr.sa_data[0],
        (unsigned char)ifr.ifr_ifru.ifru_hwaddr.sa_data[1],
        (unsigned char)ifr.ifr_ifru.ifru_hwaddr.sa_data[2],
        (unsigned char)ifr.ifr_ifru.ifru_hwaddr.sa_data[3],
        (unsigned char)ifr.ifr_ifru.ifru_hwaddr.sa_data[4],
        (unsigned char)ifr.ifr_ifru.ifru_hwaddr.sa_data[5]);
#endif
    memcpy(pHwAddr, ifr.ifr_hwaddr.sa_data, 6);
    memcpy(thiz->hwAddr, ifr.ifr_hwaddr.sa_data, 6);
  }

  if((ioctl(sd, SIOCGIFINDEX, &ifr)) == -1)
  {     
    printf("Error getting Interface index !\n");
    exit(-1);
  }

  thiz->ifindex = ifr.ifr_ifindex;

  /* Bind our raw socket to this interface */

  sll.sll_family        = PF_PACKET;
  sll.sll_ifindex       = ifr.ifr_ifindex;
  sll.sll_protocol      = htons(ETH_P_IP);

  if((bind(sd, (struct sockaddr *)&sll, sizeof(sll)))== -1)
  {
    printf("Error binding raw socket to interface\n");
    exit(-1);
  }

  thiz->hRecvSocketIpv4 = sd;
  thiz->hSdp            = hSdp;


  /*
   * Create Socket for listening ARP requests
   */
  sd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));

  if (sd < 0)
  {
    NW_IP_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
    NW_ASSERT(0);
  }

  bzero(&sll, sizeof(sll));
  bzero(&ifr, sizeof(ifr));

  /* First Get the Interface Index  */

  strncpy((char *)ifr.ifr_name, (const char*)device, IFNAMSIZ);
  if((ioctl(sd, SIOCGIFINDEX, &ifr)) == -1)
  {     
    printf("Error getting Interface index !\n");
    exit(-1);
  }

  /* Bind our raw socket to this interface */

  sll.sll_family        = PF_PACKET;
  sll.sll_ifindex       = ifr.ifr_ifindex;
  sll.sll_protocol      = htons(ETH_P_ARP);

  if((bind(sd, (struct sockaddr *)&sll, sizeof(sll)))== -1)
  {
    printf("Error binding raw socket to interface\n");
    exit(-1);
  }

  thiz->hRecvSocketArp     = sd;

  /*
   * Create socket sending data to L2
   */
  if((send_sd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW))== -1)
  {
    printf("Error creating raw socket: send fd");
    exit(1);
  }

  int yes = 1;
  if(setsockopt(send_sd, IPPROTO_IP, IP_HDRINCL, &yes, sizeof(yes)) < 0 ) {
    perror("Error in Setting socket option:");
    exit(1);
  }

  thiz->hSendSocket     = send_sd;

  return NW_OK;
}

NwRcT nwIpv4IfGetSelectionObjectIpv4(NwIpv4IfT* thiz, NwU32T *pSelObj)
{
  *pSelObj = thiz->hRecvSocketIpv4;
  return NW_OK;
}

NwRcT nwIpv4IfGetSelectionObjectArp(NwIpv4IfT* thiz, NwU32T *pSelObj)
{
  *pSelObj = thiz->hRecvSocketArp;
  return NW_OK;
}

void NW_EVT_CALLBACK(nwIpv4IfArpDataIndicationCallback)
{
  NwRcT         rc;
  NwS32T        bytesRead;
  NwArpPacketT  arpReq;
  NwIpv4IfT* thiz = (NwIpv4IfT*) arg;


  bytesRead = recvfrom(thiz->hRecvSocketArp, &arpReq, sizeof(NwArpPacketT), 0, NULL, NULL);
  if(bytesRead > 0)
  {
    NW_IP_LOG(NW_LOG_LEVEL_DEBG, "Received ARP message of length %u", bytesRead);
    if(arpReq.opCode == htons(0x0001) &&
      (*((NwU32T*)arpReq.targetIpAddr) != *((NwU32T*)arpReq.senderIpAddr)))
    {
      nwLogHexDump((NwU8T*)&arpReq, bytesRead);
      rc = nwSdpProcessIpv4DataInd(thiz->hSdp, 0, (NwU8T*)(&arpReq), (bytesRead));
    }
  }
  else
  {
    NW_IP_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
  }
  return;
}

void NW_EVT_CALLBACK(nwIpv4IfDataIndicationCallback)
{
  NwRcT         rc;
  NwU8T         ipDataBuf[MAX_IP_PAYLOAD_LEN];
  NwS32T        bytesRead;
  NwIpv4IfT* thiz = (NwIpv4IfT*) arg;

  bytesRead = recvfrom(thiz->hRecvSocketIpv4, ipDataBuf, MAX_IP_PAYLOAD_LEN , 0, NULL, NULL);
  if(bytesRead > 0)
  {
    NW_IP_LOG(NW_LOG_LEVEL_DEBG, "Received IP message of length %u", bytesRead);
    nwLogHexDump((NwU8T*)ipDataBuf, bytesRead);
    rc = nwSdpProcessIpv4DataInd(thiz->hSdp, 0, (ipDataBuf), (bytesRead));
  }
  else
  {
    NW_IP_LOG(NW_LOG_LEVEL_ERRO, "%s", strerror(errno));
  }
}

NwRcT nwIpv4IfIpv4DataReq(NwSdpHandleT hThiz,
    NwU8T* dataBuf,
    NwU32T dataSize)
{
  struct sockaddr_in peerAddr;
  NwS32T bytesSent;
  NwIpv4IfT* thiz = (NwIpv4IfT*) hThiz;

  peerAddr.sin_family       = AF_INET;
  memset(peerAddr.sin_zero, '\0', sizeof (peerAddr.sin_zero));
  peerAddr.sin_addr.s_addr = *((NwU32T*)(dataBuf + 16));

  nwLogHexDump((NwU8T*)dataBuf, dataSize);

  bytesSent = sendto (thiz->hSendSocket, dataBuf, dataSize, 0, (struct sockaddr *) &peerAddr, sizeof(struct sockaddr));

  if(bytesSent < 0)
  {
    NW_IP_LOG(NW_LOG_LEVEL_ERRO, "IP PDU send error - %s", strerror(errno));
  }
  return NW_OK;
}

NwRcT nwIpv4IfArpDataReq(NwSdpHandleT       hThiz,
                     NwU16T             opCode,
                     NwU8T              *pTargetMac,
                     NwU8T              *pTargetIpAddr,
                     NwU8T              *pSenderIpAddr)
{
  NwU32T                bytesSent;
  struct sockaddr_ll    sa;
  NwArpPacketT          arpRsp;
  NwIpv4IfT* thiz = (NwIpv4IfT*) hThiz;

  sa.sll_family         = AF_PACKET;
  sa.sll_ifindex        = thiz->ifindex;
  sa.sll_protocol       = htons(ETH_P_ARP);
  sa.sll_hatype         = ARPHRD_ETHER;
  sa.sll_halen          = ETH_ALEN;
  sa.sll_pkttype        = PACKET_BROADCAST;

  memcpy(arpRsp.dstMac, pTargetMac, ETH_ALEN);
  memcpy(arpRsp.srcMac, thiz->hwAddr, ETH_ALEN);
  arpRsp.protocol       = htons(ETH_P_ARP);

  arpRsp.hwType         = htons(ARPHRD_ETHER);
  arpRsp.protoType      = htons(ETH_P_IP);

  arpRsp.hwAddrLen      = ETH_ALEN;
  arpRsp.protoAddrLen   = 0x04;
  arpRsp.opCode         = htons(opCode);

  memcpy(arpRsp.senderMac, thiz->hwAddr, 6);
  memcpy(arpRsp.senderIpAddr, pSenderIpAddr, 4);

  if(opCode == ARPOP_REQUEST)
    memset(arpRsp.targetMac, 0x00, 6);
  else
    memcpy(arpRsp.targetMac, pTargetMac, 6);

  memcpy(arpRsp.targetIpAddr, pTargetIpAddr, 4);

  nwLogHexDump((NwU8T*)&arpRsp, sizeof(arpRsp));
  bytesSent = sendto (thiz->hRecvSocketArp, (void*)&arpRsp, sizeof(NwArpPacketT), 0, (struct sockaddr *) &sa, sizeof(sa));
  if(bytesSent < 0)
  {
    NW_IP_LOG(NW_LOG_LEVEL_ERRO, "ARP PDU send error - %s", strerror(errno));
  }

  return NW_OK;
}

#ifdef __cplusplus
}
#endif



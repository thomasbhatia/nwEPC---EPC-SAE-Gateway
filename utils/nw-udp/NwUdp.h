
#include "NwEvt.h"
#include "NwTypes.h"
#include "NwGtpv2c.h"

#ifndef __NW_UDP_H__
#define __NW_UDP_H__

/*---------------------------------------------------------------------------
 *                         U D P     E N T I T Y 
 *--------------------------------------------------------------------------*/

#define NW_GTPC_UDP_PORT                                        (2123)
#define MAX_UDP_PAYLOAD_LEN                                     (4096)

typedef struct
{
  NwU32T        hSocket;
  NwU32T        ipAddr;
  NwEventT      ev;
  NwGtpv2cStackHandleT hGtpcStack;
} NwUdpT;


NwRcT 
nwUdpDataReq(NwGtpv2cUdpHandleT udpHandle,
             NwU8T* dataBuf,
             NwU32T dataSize,
             NwU32T peerIpAddr,
             NwU32T peerIpPort);

NwRcT nwUdpInit(NwUdpT* thiz, NwU32T ipAddr, NwGtpv2cStackHandleT hGtpcStack);
NwRcT nwUdpDestroy(NwUdpT* thiz);

#endif

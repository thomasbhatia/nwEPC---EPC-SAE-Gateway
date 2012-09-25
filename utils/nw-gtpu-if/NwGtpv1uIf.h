
#include "NwTypes.h"
#include "NwError.h"
#include "NwGtpv1u.h"
#include "NwSdp.h"
#include "NwEvt.h"
#ifndef __NW_GTPV1U_IF_H__
#define __NW_GTPV1U_IF_H__

/*---------------------------------------------------------------------------
 *                         U D P     E N T I T Y 
 *--------------------------------------------------------------------------*/

#define NW_GTPU_UDP_PORT                                                (2152)
#define MAX_GTPU_PAYLOAD_LEN                                            (4096)

typedef struct
{
  NwU32T        hSocket;
  NwU32T        ipAddr;
  NwEventT      ev;
  NwSdpHandleT  hSdp;
} NwGtpv1uIfT;


NwRcT nwGtpv1uIfInitialize(NwGtpv1uIfT* thiz, NwU32T ipAddr, NwSdpHandleT hSdp);
NwRcT nwGtpv1uIfDestroy(NwGtpv1uIfT* thiz);

NwRcT nwGtpv1uIfGetSelectionObject(NwGtpv1uIfT* thiz, NwU32T *pSelObj);

void NW_EVT_CALLBACK(nwGtpv1uIfDataIndicationCallback);

NwRcT nwGtpv1uIfDataReq(NwGtpv1uUdpHandleT udpHandle,
             NwU8T* dataBuf,
             NwU32T dataSize,
             NwU32T peerIpAddr,
             NwU32T peerIpPort);

#endif

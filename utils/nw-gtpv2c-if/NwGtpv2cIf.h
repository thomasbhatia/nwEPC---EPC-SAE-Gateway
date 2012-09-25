
#ifndef __NW_GTPV2C_IF_H__
#define __NW_GTPV2C_IF_H__

#include "NwEvt.h"
#include "NwTypes.h"
#include "NwGtpv2c.h"

/*---------------------------------------------------------------------------
 *                         U D P     E N T I T Y 
 *--------------------------------------------------------------------------*/

#define NW_GTPC_UDP_PORT                                                (2123)
#define MAX_GTPV2C_PAYLOAD_LEN                                          (4096)

typedef struct
{
  NwU32T        hSocket;
  NwU32T        ipAddr;
  NwGtpv2cStackHandleT hGtpcStack;
} NwGtpv2cIfT;


NwRcT 
nwGtpv2cIfInitialize(NwGtpv2cIfT* thiz, NwU32T ipAddr, NwGtpv2cStackHandleT hGtpcStack);

NwRcT 
nwGtpv2cIfDestroy(NwGtpv2cIfT* thiz);

NwRcT 
nwGtpv2cIfGetSelectionObject(NwGtpv2cIfT* thiz, NwU32T *pSelObj);

void NW_EVT_CALLBACK(nwGtpv2cIfDataIndicationCallback);

NwRcT
nwGtpv2cIfDataReq(NwGtpv2cUdpHandleT udpHandle,
             NwU8T* dataBuf,
             NwU32T dataSize,
             NwU32T peerIpAddr,
             NwU32T peerIpPort);


#endif

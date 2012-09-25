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

#include <arpa/inet.h>

#include "NwTypes.h"
#include "NwIpv4PoolMgr.h"
#include "NwIpv4PoolMgrLog.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NwIpv4DescriptorS
{
  struct NwIpv4DescriptorS *next;
  NwU32T index;
} NwIpv4DescriptorT;

typedef struct
{
  NwU32T startIp;
  NwU32T endIp;
  NwU32T mask;
  void** pIpAddrTbl;
  NwIpv4DescriptorT* pFreePool;
} NwIpv4PoolMgrT;

NwIpv4PoolMgrHandleT
nwIpv4PoolMgrNew(NwU32T startIp, NwU32T endIp, NwU32T mask)
{
  NwU32T i;
  NwIpv4PoolMgrT* thiz;
  thiz = (NwIpv4PoolMgrT*) malloc (sizeof(NwIpv4PoolMgrT));

  if(thiz)
  {
    thiz->startIp       = startIp;
    thiz->endIp         = endIp;
    thiz->mask          = mask;
    thiz->pFreePool     = NULL;
    thiz->pIpAddrTbl    = (void**) malloc((endIp - startIp + 1) * sizeof(NwIpv4DescriptorT*));

    for(i = 0; i <= (endIp - startIp); i++)
    {
      thiz->pIpAddrTbl[i]         = (void*) malloc (sizeof(NwIpv4DescriptorT));
      ((NwIpv4DescriptorT*)thiz->pIpAddrTbl[i])->index  = i;
      ((NwIpv4DescriptorT*)thiz->pIpAddrTbl[i])->next   = thiz->pFreePool;
      thiz->pFreePool             = (NwIpv4DescriptorT*) thiz->pIpAddrTbl[i];
    } 
  }

  return (NwIpv4PoolMgrHandleT) thiz;
}

NwRcT
nwIpv4PoolMgrDelete(NwIpv4PoolMgrHandleT hIpv4PoolMgr)
{
  NwIpv4PoolMgrT* thiz = (NwIpv4PoolMgrT*) hIpv4PoolMgr;
  free((void*) thiz);
  return NW_OK;
}

NwRcT
nwIpv4PoolMgrAlloc(NwIpv4PoolMgrHandleT hIpv4PoolMgr, NwU32T* ipv4Addr)
{
  NwIpv4PoolMgrT* thiz = (NwIpv4PoolMgrT*) hIpv4PoolMgr;
  while(thiz->pFreePool)
  {
    *ipv4Addr = htonl(thiz->startIp + thiz->pFreePool->index) ;
    thiz->pFreePool = thiz->pFreePool->next;
    if(((ntohl(*ipv4Addr) & 0x000000ff) != 0x00000000) &&       /* Eliminate *.*.*.0    */
        ((ntohl(*ipv4Addr) & 0x000000ff) != 0x000000ff))        /* Eliminate *.*.*.255  */
    {
      return NW_OK;
    }
  }
  NW_IPV4_POOL_MGR_LOG(NW_LOG_LEVEL_ERRO, "Cannot allocate IPv4 address. Address pool exhausted!");
  *ipv4Addr = 0;
  return NW_FAILURE;
}

NwRcT
nwIpv4PoolMgrFree(NwIpv4PoolMgrHandleT hIpv4PoolMgr, NwU32T ipv4Addr)
{
  if(ipv4Addr)
  {
    NwIpv4PoolMgrT* thiz = (NwIpv4PoolMgrT*) hIpv4PoolMgr;
    ((NwIpv4DescriptorT*)thiz->pIpAddrTbl[(ntohl(ipv4Addr) - thiz->startIp)])->next = thiz->pFreePool;
    thiz->pFreePool = ((NwIpv4DescriptorT*)thiz->pIpAddrTbl[(ntohl(ipv4Addr) - thiz->startIp)]);
  }
  return NW_OK;
}

#ifdef __cplusplus
}
#endif

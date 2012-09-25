/*----------------------------------------------------------------------------*
 *                                                                            *
 *                               n w - i p v 4                                * 
 *           I n t e r n e t    P r o t o c o l    v 4    S t a c k           *
 *                                                                            *
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

#ifndef __NW_IPv4_MSG_H__
#define __NW_IPv4_MSG_H__

#include "NwTypes.h"
#include "NwIpv4.h"

/** 
 * @file NwIpv4Msg.h
 * @brief This file defines APIs for to build new outgoing ipv4 messages and to parse incoming messages.
*/

#ifdef __cplusplus
extern "C" {
#endif

 /**
  * Allocate a ipv4 message from data buffer.
  *
  * @param[in] hIpv4StackHandle : IPv4 stack handle.
  * @param[in] pBuf: Buffer to be copied in this message.
  * @param[in] bufLen: Buffer length to be copied in this message.
  * @param[out] phMsg : Pointer to message handle.
  */

NwIpv4RcT
nwIpv4MsgFromBufferNew( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
                         NW_IN NwU8T* pBuf,
                         NW_IN NwU32T bufLen,
                         NW_OUT NwIpv4MsgHandleT *phMsg);

 /**
  * Free a ipv4 message.
  *
  * @param[in] hIpv4StackHandle : IPv4 stack handle.
  * @param[in] hMsg : Message handle.
  */

NwIpv4RcT
nwIpv4MsgDelete( NW_IN NwIpv4StackHandleT hIpv4StackHandle,
                   NW_IN NwIpv4MsgHandleT hMsg);

/**
 * Dump the contents of Ipv4 mesasge.
 *
 * @param[in] hMsg : Handle to ipv4 message.
 * @param[in] fp: Pointer to output file.
 */

NwIpv4RcT
nwIpv4MsgHexDump(NwIpv4MsgHandleT hMsg, FILE* fp);

#ifdef __cplusplus
}
#endif

#endif /* __NW_TYPES_H__ */


/*--------------------------------------------------------------------------*
 *                      E N D     O F    F I L E                            *
 *--------------------------------------------------------------------------*/


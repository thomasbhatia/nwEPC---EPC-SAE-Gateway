/*----------------------------------------------------------------------------*
 *                                                                            *
 *                             n w - g t p v 2 u                              * 
 *  G e n e r i c    R o u t i n g    E n c a p s u l a t i o n    S t a c k  *
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

#ifndef __NW_GRE_MSG_H__
#define __NW_GRE_MSG_H__

#include "NwTypes.h"
#include "NwGre.h"

/** 
 * @file NwGreMsg.h
 * @brief This file defines APIs for to build new outgoing gre messages and to parse incoming messages.
*/

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------*
 *   G T P V 2 C     I E    D A T A - T Y P E      D E F I N I T I O N S    *
 *--------------------------------------------------------------------------*/

#pragma pack(1)

typedef struct NwGreIeTv1
{
  NwU8T  t;
  NwU16T l;
  NwU8T  i;
  NwU8T  v;
} NwGreIeTv1T;

typedef struct NwGreIeTv2
{
  NwU8T  t;
  NwU16T l;
  NwU8T  i;
  NwU8T  v;
} NwGreIeTv2T;

typedef struct NwGreIeTv4
{
  NwU8T  t;
  NwU16T l;
  NwU8T  i;
  NwU32T  v;
} NwGreIeTv4T;

typedef struct NwGreIeTlv
{
  NwU8T  t;
  NwU16T l;
  NwU8T  i;
} NwGreIeTlvT;

#pragma pack()


 /**
  * Allocate a GPDU gre message.
  *
  * @param[in] hGreStackHandle : GRE stack handle.
  * @param[in] teidPresent : TEID is present flag.
  * @param[in] teid : TEID for this message.
  * @param[in] seqNum : Sequence number for this message.
  * @param[out] phMsg : Pointer to message handle.
  */

NwRcT
nwGreMsgNew( NW_IN NwGreStackHandleT hGreStackHandle,
                NW_IN NwU8T     keyPresent,
                NW_IN NwU8T     seqNumPresent,
                NW_IN NwU8T     csumPresent,
                NW_IN NwU8T     msgType,
                NW_IN NwU8T     teid,
                NW_IN NwU16T    seqNum,
                NW_IN NwU8T     npduNum,
                NW_IN NwU8T     nextExtHeader,
                NW_OUT NwGreMsgHandleT *phMsg);


 /**
  * Allocate a gre message.
  *
  * @param[in] hGreStackHandle : GRE stack handle.
  * @param[in] teid : TEID for this message.
  * @param[in] keyPresent : Sequence number flag for this message.
  * @param[in] seqNum : Sequence number for this message.
  * @param[in] pTpdu: T-PDU for this message.
  * @param[in] tpduLength: T-PDU length for this message.
  * @param[out] phMsg : Pointer to message handle.
  */

NwRcT
nwGreGpduMsgNew( NW_IN NwGreStackHandleT hGreStackHandle,
                NW_IN NwU8T     csumPresent,
                NW_IN NwU8T     keyPresent,
                NW_IN NwU8T     seqNumPresent,
                NW_IN NwU32T    key,
                NW_IN NwU16T    seqNum,
                NW_IN NwU8T*    tpdu,
                NW_IN NwU16T    tpduLength,
                NW_OUT NwGreMsgHandleT *phMsg);


 /**
  * Allocate a gre message from data buffer.
  *
  * @param[in] hGreStackHandle : GRE stack handle.
  * @param[in] pBuf: Buffer to be copied in this message.
  * @param[in] bufLen: Buffer length to be copied in this message.
  * @param[out] phMsg : Pointer to message handle.
  */

NwRcT
nwGreMsgFromBufferNew( NW_IN NwGreStackHandleT hGreStackHandle,
                         NW_IN NwU8T* pBuf,
                         NW_IN NwU32T bufLen,
                         NW_OUT NwGreMsgHandleT *phMsg);

 /**
  * Free a gre message.
  *
  * @param[in] hGreStackHandle : GRE stack handle.
  * @param[in] hMsg : Message handle.
  */

NwRcT
nwGreMsgDelete( NW_IN NwGreStackHandleT hGreStackHandle,
                   NW_IN NwGreMsgHandleT hMsg);

 /**
  * Set TEID for gre message.
  *
  * @param[in] hMsg : Message handle.
  * @param[in] teid: TEID value.
  */

NwRcT
nwGreMsgSetTeid(NW_IN NwGreMsgHandleT hMsg, NwU32T teid);

 /**
  * Set TEID present flag for gre message.
  *
  * @param[in] hMsg : Message handle.
  * @param[in] teidPesent: Flag boolean value.
  */

NwRcT
nwGreMsgSetTeidPresent(NW_IN NwGreMsgHandleT hMsg, NwBoolT teidPresent);

 /**
  * Set sequence for gre message.
  *
  * @param[in] hMsg : Message handle.
  * @param[in] seqNum: Flag boolean value.
  */

NwRcT
nwGreMsgSetSeqNumber(NW_IN NwGreMsgHandleT hMsg, NwU32T seqNum);

 /**
  * Get TEID present for gre message.
  *
  * @param[in] hMsg : Message handle.
  */

NwU32T
nwGreMsgGetTeid(NW_IN NwGreMsgHandleT hMsg);

 /**
  * Get TEID present for gre message.
  *
  * @param[in] hMsg : Message handle.
  */

NwBoolT
nwGreMsgGetTeidPresent(NW_IN NwGreMsgHandleT hMsg);

 /**
  * Get sequence number for gre message.
  *
  * @param[in] hMsg : Message handle.
  */

NwU32T
nwGreMsgGetSeqNumber(NW_IN NwGreMsgHandleT hMsg);

 /**
  * Get tpdu for gre message.
  *
  * @param[in] hMsg : Message handle.
  * @param[inout] pTpduBuf : Buffer to copy the T-PDU.
  * @param[out] hMsg : T-PDU length.
  */

NwU32T
nwGreMsgGetTpdu(NW_IN NwGreMsgHandleT hMsg, NwU8T* pTpduBuf, NwU32T* pTpduLength);

 /**
  * Add a gre information element of length 1 to gre mesasge.
  *
  * @param[in] hMsg : Handle to gre message.
  * @param[in] type : IE type.
  * @param[in] instance : IE instance.
  * @param[in] value : IE value.
  */

NwRcT
nwGreMsgAddIeTV1(NW_IN NwGreMsgHandleT hMsg, 
              NW_IN NwU8T       type,
              NW_IN NwU8T       instance,
              NW_IN NwU8T       value);


 /**
  * Add a gre information element of length 2 to gre mesasge.
  *
  * @param[in] hMsg : Handle to gre message.
  * @param[in] type : IE type.
  * @param[in] instance : IE instance.
  * @param[in] value : IE value.
  */

NwRcT
nwGreMsgAddIeTV2(NW_IN NwGreMsgHandleT hMsg, 
              NW_IN NwU8T       type,
              NW_IN NwU16T      length,
              NW_IN NwU8T       instance,
              NW_IN NwU16T      value);


 /**
  * Add a gre information element of length 4 to gre mesasge.
  *
  * @param[in] hMsg : Handle to gre message.
  * @param[in] type : IE type.
  * @param[in] instance : IE instance.
  * @param[in] value : IE value.
  */

NwRcT
nwGreMsgAddIeTV4(NW_IN NwGreMsgHandleT hMsg, 
              NW_IN NwU8T       type,
              NW_IN NwU16T      length,
              NW_IN NwU8T       instance,
              NW_IN NwU32T      value);


 /**
  * Add a gre information element of variable length to gre mesasge.
  *
  * @param[in] hMsg : Handle to gre message.
  * @param[in] type : IE type.
  * @param[in] length : IE length.
  * @param[in] instance : IE instance.
  * @param[in] value : IE value.
  */

NwRcT
nwGreMsgAddIe(NW_IN NwGreMsgHandleT hMsg, 
              NW_IN NwU8T       type,
              NW_IN NwU16T      length,
              NW_IN NwU8T       instance,
              NW_IN NwU8T*      pVal);

/**
 * Dump the contents of gre mesasge.
 *
 * @param[in] hMsg : Handle to gre message.
 * @param[in] fp: Pointer to output file.
 */

NwRcT
nwGreMsgHexDump(NwGreMsgHandleT hMsg, FILE* fp);

#ifdef __cplusplus
}
#endif

#endif /* __NW_TYPES_H__ */


/*--------------------------------------------------------------------------*
 *                      E N D     O F    F I L E                            *
 *--------------------------------------------------------------------------*/


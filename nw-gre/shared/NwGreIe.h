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

/** 
 * @file NwGreIe.h
 * @brief This header file contains Information Element definitions for GRE
 * as per 3GPP TS 29274-930.
*/

#ifndef __NW_GRE_IE_H__
#define __NW_GRE_IE_H__

/*--------------------------------------------------------------------------*
 *   G T P V 2 C    I E  T Y P E     M A C R O     D E F I N I T I O N S    *
 *--------------------------------------------------------------------------*/

#define NW_GRE_IE_RESERVED                                           (0)
#define NW_GRE_IE_IMSI                                               (1)
#define NW_GRE_IE_CAUSE                                              (2)
#define NW_GRE_IE_RECOVERY                                           (3)
#define NW_GRE_IE_APN                                                (71)
#define NW_GRE_IE_PRIVATE_EXTENSION                                  (255)


/*--------------------------------------------------------------------------*
 *   G T P V 2 C      C A U S E      V A L U E     D E F I N I T I O N S    *
 *--------------------------------------------------------------------------*/

#define NW_GRE_CAUSE_REQUEST_ACCEPTED                                (16) 
#define NW_GRE_CAUSE_MANDATORY_IE_MISSING                            (70) 

#endif  /* __NW_GRE_IE_H__ */

/*--------------------------------------------------------------------------*
 *                      E N D     O F    F I L E                            *
 *--------------------------------------------------------------------------*/


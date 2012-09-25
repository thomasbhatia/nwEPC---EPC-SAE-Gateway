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

#ifndef __NW_GRE_LOG_H__
#define __NW_GRE_LOG_H__

#include <stdio.h>
#include <libgen.h>

/** 
 * @file NwGreLog.h 
 * @brief This header contains logging related definitions.
*/

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------
 * Log Macro Definition
 *--------------------------------------------------------------------------*/

#define NW_LOG(_greHandle, _logLevel, ...)                           \
  do {                                                                  \
    if(((NwGreStackT*)(_greHandle))->logLevel >= _logLevel)       \
    {                                                                   \
      char _logBuf[1024];                                               \
      snprintf(_logBuf, 1024, __VA_ARGS__);                             \
      ((NwGreStackT*)(_greHandle))->logMgr.logReqCallback(((NwGreStackT*)_greHandle)->logMgr.logMgrHandle, _logLevel, __FILE__, __LINE__, _logBuf);\
    }                                                                   \
  } while(0)

#define NW_ENTER(_greHandle)                                       \
  do {                                                                  \
    NW_LOG(_greHandle, NW_LOG_LEVEL_DEBG, "Entering '%s'", __func__);\
  } while(0)

#define NW_LEAVE(_greHandle)                                       \
  do {                                                                  \
    NW_LOG(_greHandle, NW_LOG_LEVEL_DEBG, "Leaving '%s'", __func__);\
  } while(0)

#ifdef __cplusplus
}
#endif

#endif /* __NW_GRE_LOG_H__ */


/*--------------------------------------------------------------------------*
 *                      E N D     O F    F I L E                            *
 *--------------------------------------------------------------------------*/


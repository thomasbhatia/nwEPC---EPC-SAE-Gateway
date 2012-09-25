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

#ifndef __NW_EVT_H__
#define __NW_EVT_H__
/** 
 * @file NwEvt.h
 * @brief 
*/

#ifdef __WITH_LIBEVENT__

#include <event.h>

typedef struct event                    NwEventT;

#define NW_EVT_READ                     (EV_READ)
#define NW_EVT_PERSIST                  (EV_PERSIST)
#define NW_EVT_CALLBACK(__cbFunc)       __cbFunc(int fd, short event, void *arg)
#define NW_TMR_CALLBACK(__cbFunc)       __cbFunc(int fd, short event, void *arg)

#define NW_EVT_INIT                     event_init
#define NW_EVT_LOOP                     event_dispatch

#define NW_EVENT_ADD(__ev, __evSelObj, __evCallback, __evCallbackArg, __evFlags)        \
  do {                                                                                  \
    event_set(&(__ev), __evSelObj, __evFlags, __evCallback, __evCallbackArg);           \
    event_add(&(__ev), NULL);                                                           \
  } while(0)

#else

#include "NwEvent.h"

#define NW_EVT_READ                     (NW_EVENT_READ)
#define NW_EVT_PERSIST                  (0x00000000)
#define NW_EVT_CALLBACK(__cbFunc)       __cbFunc(void *arg)
#define NW_TMR_CALLBACK(__cbFunc)       __cbFunc(void *arg)

#define NW_EVT_INIT                     nwEventInitialize 
#define NW_EVT_LOOP                     nwEventLoop 

#define NW_EVENT_ADD(__ev, __evSelObj, __evCallback, __evCallbackArg, __evFlags)        \
  do {                                                                                  \
    nwEventAdd(&(__ev), __evSelObj, __evCallback, __evCallbackArg, __evFlags);          \
  } while(0)

#endif


#endif

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
 * @file NwLog.h
 * @brief This file defines log manager entity. 
*/

#include <stdio.h>

#include "NwTypes.h"
#include "NwError.h"
#include "NwLog.h"

#ifndef __NW_LOG_MGR_H__
#define __NW_LOG_MGR_H__

typedef struct
{
  NwU8T   compName[32];
  NwU32T  compInst;
  NwU32T  logLevel;
  NwU8T   logDir[32];
  FILE*   fp;           /* Log File Pointer */
} NwLogMgrT;

extern NwLogMgrT _gLogMgr;

extern NwLogMgrT*
nwLogMgrGetInstance();

extern NwRcT 
nwLogMgrInit(NwLogMgrT* thiz, NwU8T* strCompName, NwU32T compInst);

extern NwRcT 
nwLogMgrGetLogLevel(NwLogMgrT* thiz);

extern NwRcT 
nwLogHexDump(NwU8T* data, NwU32T size);

extern NwRcT 
nwLogMgrLog(NwLogMgrT* thiz,
                  NwCharT*   logModuleStr,
                  NwU8T      logLevel,
                  NwCharT*   fileNameStr,
                  NwU32T     line,
                  NwCharT*   logStr);
#endif

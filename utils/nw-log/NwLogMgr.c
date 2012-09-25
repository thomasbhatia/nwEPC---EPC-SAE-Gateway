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
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

#include "NwTypes.h"
#include "NwError.h"
#include "NwLog.h"
#include "NwLogMgr.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NW_LOG_TO_FILE
#undef NW_LOG_TO_FILE
#endif

static
NwCharT* gLogLevelStr[] = {"EMER", "ALER", "CRIT",  "ERRO", "WARN", "NOTI", "INFO", "DEBG"};

static
NwU32T logCount = 1;
/*---------------------------------------------------------------------------
 *                       L O G M G R     E N T I T Y 
 *--------------------------------------------------------------------------*/

#ifdef NW_LOG_TO_FILE
static 
NwRcT nwLogMgrFileOpen(NwLogMgrT* thiz)
{
  NwU8T fileStr[128];
  struct timeval tv;
  struct tm *pTm;

  gettimeofday( &tv, NULL );
  pTm = localtime ( (const time_t*)&tv.tv_sec );

  strcpy((char*)fileStr, thiz->logDir);
  sprintf((char*) (fileStr + strlen((char*)fileStr)), "/%s_%u", thiz->compName, thiz->compInst);
  strftime ((char*) (fileStr + strlen((char*)fileStr)), 128, "_%d%b%Y_%T", pTm );
  strcat((char*)fileStr, ".log");
  thiz->fp = fopen((char*)fileStr, "a");
  return NW_OK;
}
#endif

NwLogMgrT _gLogMgr;

NwLogMgrT*
nwLogMgrGetInstance()
{
  return &(_gLogMgr);
}

NwRcT nwLogMgrInit(NwLogMgrT* thiz, NwU8T* strCompName, NwU32T compInst)
{
  char *logLevelStr;
  char createLogDirCmd[64];

  logLevelStr = getenv ("NW_LOG_LEVEL");

  if(logLevelStr == NULL)
  {
    thiz->logLevel = NW_LOG_LEVEL_INFO;
  }
  else
  {
    if(strncmp(logLevelStr, "EMER",4) == 0)
      thiz->logLevel = NW_LOG_LEVEL_EMER;
    else if(strncmp(logLevelStr, "ALER",4) == 0)
      thiz->logLevel = NW_LOG_LEVEL_ALER;
    else if(strncmp(logLevelStr, "CRIT",4) == 0)
      thiz->logLevel = NW_LOG_LEVEL_CRIT;
    else if(strncmp(logLevelStr, "ERRO",4) == 0)
      thiz->logLevel = NW_LOG_LEVEL_ERRO ;
    else if(strncmp(logLevelStr, "WARN",4) == 0)
      thiz->logLevel = NW_LOG_LEVEL_WARN;
    else if(strncmp(logLevelStr, "NOTI",4) == 0)
      thiz->logLevel = NW_LOG_LEVEL_NOTI;
    else if(strncmp(logLevelStr, "INFO",4) == 0)
      thiz->logLevel = NW_LOG_LEVEL_INFO;
    else if(strncmp(logLevelStr, "DEBG",4) == 0)
      thiz->logLevel = NW_LOG_LEVEL_DEBG;

  }

  strcpy((char*)thiz->compName, (char*)strCompName);
  thiz->compInst = compInst;

#ifdef NW_LOG_TO_FILE
  strcpy(thiz->logDir, "/opt/nw-epc/log/");
  strcat(thiz->logDir, (char*)strCompName);

  strcpy(createLogDirCmd, "mkdir -p ");
  strcat(createLogDirCmd, thiz->logDir); 

  system(createLogDirCmd);
  return nwLogMgrFileOpen(thiz);
#else
  return NW_OK;
#endif
}

NwRcT nwLogMgrGetLogLevel(NwLogMgrT* thiz)
{
  return thiz->logLevel;
}

NwRcT nwLogMgrLog(NwLogMgrT* thiz,
                  NwCharT*   logModuleStr,
                  NwU8T      logLevel,
                  NwCharT*   fileNameStr,
                  NwU32T     line,
                  NwCharT*   logStr)
{
  NwRcT rc;
  NwU8T timeStr[128];
  struct timeval tv;
  struct tm *pTm;

  gettimeofday( &tv, NULL );
  pTm = localtime ( (const time_t*)&tv.tv_sec );
  strftime ((char*) timeStr, 128, "%d/%m/%Y %T", pTm );
  sprintf((char*)(timeStr + strlen((char*)timeStr)), ".%06u", (unsigned int)tv.tv_usec);
#ifdef NW_LOG_TO_FILE
  fprintf(thiz->fp, "[%06u] %s %s %s - %s <%s,%u>\n", logCount, timeStr, logModuleStr, gLogLevelStr[logLevel], logStr, fileNameStr, line);
  if(logCount % 5000 == 0)
  {
    fclose(thiz->fp);
    rc = nwLogMgrFileOpen(thiz);
  }
#else
  printf("[%06u] %s %s %s - %s <%s,%u>\n", logCount, timeStr, logModuleStr, gLogLevelStr[logLevel], logStr, fileNameStr, line);
  rc = NW_OK;
#endif
  logCount++;
  return rc;
}


NwRcT nwLogHexDump(NwU8T* data, NwU32T size)
{
  FILE* fp = stdout;
  unsigned char *p = (unsigned char*)data;
  unsigned char c;
  int n;
  char bytestr[4] = {0};
  char addrstr[10] = {0};
  char hexstr[ 16*3 + 5] = {0};
  char charstr[16*1 + 5] = {0};

  if(_gLogMgr.logLevel == NW_LOG_LEVEL_DEBG)
  {
    fprintf((FILE*)fp, "\n");
    for(n=1;n<=size;n++) {
      if (n%16 == 1) {
        /* store address for this line */
        snprintf(addrstr, sizeof(addrstr), "%.4x",
            ((unsigned int)p-(unsigned int)data) );
      }

      c = *p;
      if (isalnum(c) == 0) {
        c = '.';
      }

      /* store hex str (for left side) */
      snprintf(bytestr, sizeof(bytestr), "%02X ", *p);
      strncat(hexstr, bytestr, sizeof(hexstr)-strlen(hexstr)-1);

      /* store char str (for right side) */
      snprintf(bytestr, sizeof(bytestr), "%c", c);
      strncat(charstr, bytestr, sizeof(charstr)-strlen(charstr)-1);
      if(n%16 == 0) {
        /* line completed */
        fprintf((FILE*)fp, "[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
        hexstr[0] = 0;
        charstr[0] = 0;
      } else if(n%8 == 0) {
        /* half line: add whitespaces */
        strncat(hexstr, "  ", sizeof(hexstr)-strlen(hexstr)-1);
        strncat(charstr, " ", sizeof(charstr)-strlen(charstr)-1);
      }
      p++; /* next byte */
    }

    if (strlen(hexstr) > 0) {
      /* print rest of buffer if not empty */
      fprintf((FILE*)fp, "[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);

    }
    fprintf((FILE*)fp, "\n");
  }

  return NW_OK;

}

#ifdef __cplusplus
}
#endif

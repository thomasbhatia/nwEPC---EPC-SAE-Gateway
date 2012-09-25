/*----------------------------------------------------------------------------*
 *                                                                            *
 *         M I N I M A L I S T I C    L O G M G R     E N T I T Y             *
 *                                                                            *
 *                    Copyright (C) 2010 Amit Chawre.                         *
 *                                                                            *
 *----------------------------------------------------------------------------*/

/** 
 * @file hello-world.c
 * @brief This file contains example of a minimalistic log manager entity.
*/

#include <stdio.h>
#include <assert.h>
#include "NwEvt.h"
#include "NwGre.h"

#include "NwMiniLogMgrEntity.h"

#ifdef __cplusplus 
extern "C" {
#endif

NwU32T g_log_level = NW_LOG_LEVEL_INFO;

/*---------------------------------------------------------------------------
 * Public functions
 *--------------------------------------------------------------------------*/

NwRcT nwMiniLogMgrInit(NwMiniLogMgrT* thiz, NwU32T logLevel )
{
  thiz->logLevel = logLevel;
  return NW_OK;
}

NwRcT nwMiniLogMgrSetLogLevel(NwMiniLogMgrT* thiz, NwU32T logLevel)
{
  thiz->logLevel = logLevel;
}

NwRcT nwMiniLogMgrLogRequest (NwGreLogMgrHandleT hLogMgr,
    NwU32T logLevel,
    NwCharT* file,
    NwU32T line,
    NwCharT* logStr)
{
  NwMiniLogMgrT* thiz = (NwMiniLogMgrT*) hLogMgr;
  if(thiz->logLevel >= logLevel)
    printf("NWGREU-STK  %s - %s <%s,%u>\n", greLogLevelStr[logLevel], logStr, basename(file), line);
  return NW_OK;
}

#ifdef __cplusplus 
}
#endif


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
#include "NwTypes.h"
#include "NwIpv4Error.h"
#include "NwIpv4.h"

#ifndef NW_ASSERT
#define NW_ASSERT assert
#endif 

#ifndef __NW_MINI_LOG_MGR_H__
#define __NW_MINI_LOG_MGR_H__

extern NwU32T g_log_level;


#define NW_LOG( _logLevel, ...)                                         \
  do {                                                                  \
    if(g_log_level >= _logLevel)                                        \
    {                                                                   \
      char _logStr[1024];                                               \
      snprintf(_logStr, 1024, __VA_ARGS__);                             \
      printf("NWIPv4-APP  %s - %s <%s,%u>\n", ipv4LogLevelStr[_logLevel], _logStr, basename(__FILE__), __LINE__);\
    }                                                                   \
  } while(0)

typedef struct
{
  NwU8T  logLevel;
} NwMiniLogMgrT;

#ifdef __cplusplus
extern "C" {
#endif

NwIpv4RcT nwMiniLogMgrLogRequest (NwIpv4LogMgrHandleT logMgrHandle,
    NwU32T logLevel,
    NwCharT* file,
    NwU32T line,
    NwCharT* logStr);

#ifdef __cplusplus 
}
#endif

#endif

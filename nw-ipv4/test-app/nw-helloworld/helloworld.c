/*----------------------------------------------------------------------------*
 *                                                                            *
 *                               n w - i p v 4                                * 
 *           I n t e r n e t    P r o t o c o l    v 4    S t a c k           *
 *                                                                            *
 *           M I N I M A L I S T I C     D E M O N S T R A T I O N            *
 *                                                                            *
 *                    Copyright (C) 2010 Amit Chawre.                         *
 *                                                                            *
 *----------------------------------------------------------------------------*/


/** 
 * @file hello-world.c
 * @brief This is a test program demostrating usage of nw-Ipv4 library.
*/

#include <stdio.h>
#include <assert.h>
#include "NwEvt.h"

#include "NwMiniLogMgrEntity.h"
#include "NwMiniTmrMgrEntity.h"
#include "NwMiniLlpEntity.h"
#include "NwMiniUlpEntity.h"

#ifndef NW_ASSERT
#define NW_ASSERT assert
#endif 

/*---------------------------------------------------------------------------
 *                T H E      M A I N      F U N C T I O N 
 *--------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
  NwIpv4RcT rc; 
  char*                         logLevelStr;
  NwU32T                        logLevel;
  NwU32T                        num_of_connections;

  NwIpv4StackHandleT          hIpv4Stack = 0;
  NwMiniUlpEntityT              ulpObj;
  NwMiniLlpEntityT              llpObj;
  NwMiniLogMgrT                 logObj;
  NwIpv4UlpEntityT            ulp;
  NwIpv4LlpEntityT            llp;
  NwIpv4TimerMgrEntityT       tmrMgr;
  NwIpv4LogMgrEntityT         logMgr;

  if(argc != 4)
  {
    printf("Usage: %s <num-of-connections> <local-ip> <peer-ip>\n", argv[0]);
    exit(0);
  }


  logLevelStr = getenv ("NW_LOG_LEVEL");

  if(logLevelStr == NULL)
  {
    logLevel = NW_LOG_LEVEL_INFO;
  }
  else
  {
    if(strncmp(logLevelStr, "EMER",4) == 0)
      logLevel = NW_LOG_LEVEL_EMER;
    else if(strncmp(logLevelStr, "ALER",4) == 0)
      logLevel = NW_LOG_LEVEL_ALER;
    else if(strncmp(logLevelStr, "CRIT",4) == 0)
      logLevel = NW_LOG_LEVEL_CRIT;
    else if(strncmp(logLevelStr, "ERRO",4) == 0)
      logLevel = NW_LOG_LEVEL_ERRO ;
    else if(strncmp(logLevelStr, "WARN",4) == 0)
      logLevel = NW_LOG_LEVEL_WARN;
    else if(strncmp(logLevelStr, "NOTI",4) == 0)
      logLevel = NW_LOG_LEVEL_NOTI;
    else if(strncmp(logLevelStr, "INFO",4) == 0)
      logLevel = NW_LOG_LEVEL_INFO;
    else if(strncmp(logLevelStr, "DEBG",4) == 0)
      logLevel = NW_LOG_LEVEL_DEBG;
  }

  /*---------------------------------------------------------------------------
   *  Initialize event library
   *--------------------------------------------------------------------------*/

  NW_EVT_INIT();

  /*---------------------------------------------------------------------------
   *  Initialize Log Manager 
   *--------------------------------------------------------------------------*/

  nwMiniLogMgrInit(&logObj, logLevel);

  /*---------------------------------------------------------------------------
   *  Create IPv4 Stack Instance
   *--------------------------------------------------------------------------*/

  rc = nwIpv4Initialize(&hIpv4Stack);

  if(rc != NW_IPv4_OK)
  {
    NW_LOG(NW_LOG_LEVEL_ERRO, "Failed to create IPv4 stack instance. Error '%u' occured", rc);
    exit(1);
  }

  NW_LOG(NW_LOG_LEVEL_INFO, "Ipv4 Stack Handle '%X' Creation Successful!", hIpv4Stack);
  
  /*---------------------------------------------------------------------------
   * Set up Ulp Entity 
   *--------------------------------------------------------------------------*/

  rc = nwMiniUlpInit(&ulpObj, hIpv4Stack);
  NW_ASSERT( rc == NW_IPv4_OK );

  ulp.hUlp = (NwIpv4UlpHandleT) &ulpObj;
  ulp.ulpReqCallback = nwMiniUlpProcessStackReqCallback;

  rc = nwIpv4SetUlpEntity(hIpv4Stack, &ulp);
  NW_ASSERT( rc == NW_IPv4_OK );


  /*---------------------------------------------------------------------------
   * Set up Udp Entity 
   *--------------------------------------------------------------------------*/

  rc = nwMiniUdpInit(&llpObj, hIpv4Stack, (argv[2]));
  NW_ASSERT( rc == NW_IPv4_OK );

  llp.hLlp = (NwIpv4LlpHandleT) &llpObj;
  llp.llpDataReqCallback = nwMiniUdpDataReq;

  rc = nwIpv4SetLlpEntity(hIpv4Stack, &llp);
  NW_ASSERT( rc == NW_IPv4_OK );

  /*---------------------------------------------------------------------------
   * Set up Log Entity 
   *--------------------------------------------------------------------------*/

  tmrMgr.tmrMgrHandle = 0;
  tmrMgr.tmrStartCallback = nwTimerStart;
  tmrMgr.tmrStopCallback = nwTimerStop;

  rc = nwIpv4SetTimerMgrEntity(hIpv4Stack, &tmrMgr);
  NW_ASSERT( rc == NW_IPv4_OK );

  /*---------------------------------------------------------------------------
   * Set up Log Entity 
   *--------------------------------------------------------------------------*/

  logMgr.logMgrHandle   = (NwIpv4LogMgrHandleT) &logObj;
  logMgr.logReqCallback  = nwMiniLogMgrLogRequest;

  rc = nwIpv4SetLogMgrEntity(hIpv4Stack, &logMgr);
  NW_ASSERT( rc == NW_IPv4_OK );

  /*---------------------------------------------------------------------------
   * Set log level  
   *--------------------------------------------------------------------------*/

  rc = nwIpv4SetLogLevel(hIpv4Stack, logLevel);

  /*---------------------------------------------------------------------------
   *  Send Create Session Request to IPv4 Stack Instance
   *--------------------------------------------------------------------------*/

  num_of_connections = atoi(argv[1]);

  while ( num_of_connections-- )
  {
    rc = nwMiniUlpCreateConn(&ulpObj, argv[2], 1234 + num_of_connections, argv[3]);
    NW_ASSERT( rc == NW_IPv4_OK );
  }

  /*---------------------------------------------------------------------------
   * Event loop 
   *--------------------------------------------------------------------------*/

  NW_EVT_LOOP();
  NW_LOG(NW_LOG_LEVEL_ERRO, "Exit from eventloop, no events to process!");

  /*---------------------------------------------------------------------------
   *  Send Destroy Session Request to IPv4 Stack Instance
   *--------------------------------------------------------------------------*/

  rc = nwMiniUlpDestroyConn(&ulpObj);
  NW_ASSERT( rc == NW_IPv4_OK );


  /*---------------------------------------------------------------------------
   *  Destroy IPv4 Stack Instance
   *--------------------------------------------------------------------------*/

  rc = nwIpv4Finalize(hIpv4Stack);

  if(rc != NW_IPv4_OK)
  {
    NW_LOG(NW_LOG_LEVEL_ERRO, "Failed to finalize IPv4 stack instance. Error '%u' occured", rc);
  }
  else
  {
    NW_LOG(NW_LOG_LEVEL_INFO, "Ipv4 Stack Handle '%X' Finalize Successful!", hIpv4Stack);
  }


  return rc;
}

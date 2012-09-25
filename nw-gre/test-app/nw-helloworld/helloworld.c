/*----------------------------------------------------------------------------*
 *                                                                            *
 *                             n w - g t p v 2 u                              * 
 *  G e n e r i c    R o u t i n g    E n c a p s u l a t i o n    S t a c k  *
 *                                                                            *
 *           M I N I M A L I S T I C     D E M O N S T R A T I O N            *
 *                                                                            *
 *                    Copyright (C) 2010 Amit Chawre.                         *
 *                                                                            *
 *----------------------------------------------------------------------------*/


/** 
 * @file hello-world.c
 * @brief This is a test program demostrating usage of nw-gre library.
*/

#include <stdio.h>
#include <assert.h>
#include "NwEvt.h"
#include "NwGre.h"

#include "NwMiniLogMgrEntity.h"
#include "NwMiniTmrMgrEntity.h"
#include "NwMiniUdpEntity.h"
#include "NwMiniUlpEntity.h"

#ifndef NW_ASSERT
#define NW_ASSERT assert
#endif 

/*---------------------------------------------------------------------------
 *                T H E      M A I N      F U N C T I O N 
 *--------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
  NwRcT rc; 
  char*                         logLevelStr;
  NwU32T                        logLevel;
  NwU32T                        num_of_connections;

  NwGreStackHandleT          hGreStack = 0;
  NwMiniUlpEntityT              ulpObj;
  NwMiniUdpEntityT              udpObj;
  NwMiniLogMgrT                 logObj;
  NwGreUlpEntityT            ulp;
  NwGreLlpEntityT            udp;
  NwGreTimerMgrEntityT       tmrMgr;
  NwGreLogMgrEntityT         logMgr;

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
   *  Create GRE Stack Instance
   *--------------------------------------------------------------------------*/

  rc = nwGreInitialize(&hGreStack);

  if(rc != NW_OK)
  {
    NW_LOG(NW_LOG_LEVEL_ERRO, "Failed to create GRE stack instance. Error '%u' occured", rc);
    exit(1);
  }

  NW_LOG(NW_LOG_LEVEL_INFO, "Gre Stack Handle '%X' Creation Successful!", hGreStack);
  
  /*---------------------------------------------------------------------------
   * Set up Ulp Entity 
   *--------------------------------------------------------------------------*/

  rc = nwMiniUlpInit(&ulpObj, hGreStack);
  NW_ASSERT( NW_OK == rc );

  ulp.hUlp = (NwGreUlpHandleT) &ulpObj;
  ulp.ulpReqCallback = nwMiniUlpProcessStackReqCallback;

  rc = nwGreSetUlpEntity(hGreStack, &ulp);
  NW_ASSERT( NW_OK == rc );


  /*---------------------------------------------------------------------------
   * Set up Udp Entity 
   *--------------------------------------------------------------------------*/

  rc = nwMiniUdpInit(&udpObj, hGreStack, (argv[2]));
  NW_ASSERT( NW_OK == rc );

  udp.hUdp = (NwGreUdpHandleT) &udpObj;
  udp.udpDataReqCallback = nwMiniUdpDataReq;

  rc = nwGreSetUdpEntity(hGreStack, &udp);
  NW_ASSERT( NW_OK == rc );

  /*---------------------------------------------------------------------------
   * Set up Log Entity 
   *--------------------------------------------------------------------------*/

  tmrMgr.tmrMgrHandle = 0;
  tmrMgr.tmrStartCallback = nwTimerStart;
  tmrMgr.tmrStopCallback = nwTimerStop;

  rc = nwGreSetTimerMgrEntity(hGreStack, &tmrMgr);
  NW_ASSERT( NW_OK == rc );

  /*---------------------------------------------------------------------------
   * Set up Log Entity 
   *--------------------------------------------------------------------------*/

  logMgr.logMgrHandle   = (NwGreLogMgrHandleT) &logObj;
  logMgr.logReqCallback  = nwMiniLogMgrLogRequest;

  rc = nwGreSetLogMgrEntity(hGreStack, &logMgr);
  NW_ASSERT( NW_OK == rc );

  /*---------------------------------------------------------------------------
   * Set GRE log level  
   *--------------------------------------------------------------------------*/

  rc = nwGreSetLogLevel(hGreStack, logLevel);

  /*---------------------------------------------------------------------------
   *  Send Create Session Request to GRE Stack Instance
   *--------------------------------------------------------------------------*/

  num_of_connections = atoi(argv[1]);

  while ( num_of_connections-- )
  {
    rc = nwMiniUlpCreateConn(&ulpObj, argv[2], 1234 + num_of_connections, argv[3]);
    NW_ASSERT( NW_OK == rc );
  }

  /*---------------------------------------------------------------------------
   * Event loop 
   *--------------------------------------------------------------------------*/

  NW_EVT_LOOP();
  NW_LOG(NW_LOG_LEVEL_ERRO, "Exit from eventloop, no events to process!");

  /*---------------------------------------------------------------------------
   *  Send Destroy Session Request to GRE Stack Instance
   *--------------------------------------------------------------------------*/

  rc = nwMiniUlpDestroyConn(&ulpObj);
  NW_ASSERT( NW_OK == rc );


  /*---------------------------------------------------------------------------
   *  Destroy GRE Stack Instance
   *--------------------------------------------------------------------------*/

  rc = nwGreFinalize(hGreStack);

  if(rc != NW_OK)
  {
    NW_LOG(NW_LOG_LEVEL_ERRO, "Failed to finalize GRE stack instance. Error '%u' occured", rc);
  }
  else
  {
    NW_LOG(NW_LOG_LEVEL_INFO, "Gre Stack Handle '%X' Finalize Successful!", hGreStack);
  }


  return rc;
}

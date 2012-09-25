/*----------------------------------------------------------------------------*
 *                                                                            *
 *                              n w - s d p                                   * 
 *                    S o f t     D a t a     P l a n e                       *
 *                                                                            *
 *           M I N I M A L I S T I C     D E M O N S T R A T I O N            *
 *                                                                            *
 *                    Copyright (C) 2010 Amit Chawre.                         *
 *                                                                            *
 *----------------------------------------------------------------------------*/


/** 
 * @file hello-world.c
 * @brief This is a test program demostrating usage of nw-gtpv2 library.
*/

#include <stdio.h>
#include <assert.h>
#include "NwEvt.h"
#include "NwSdp.h"

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
  NwSdpRcT rc; 
  char*                         logLevelStr;
  NwU32T                        logLevel;
  NwU32T                        num_of_connections;

  NwSdpHandleT                  hSdp = 0;
  NwU32T                        hGreService = 0;
  NwU32T                        hGreInterface = 0;
  NwMiniUlpEntityT              ulpObj;
  NwMiniUdpEntityT              udpObj;
  NwMiniLogMgrT                 logObj;
  NwSdpUlpEntityT               ulp;
  NwSdpUdpEntityT               udp;
  NwSdpTimerMgrEntityT          tmrMgr;
  NwSdpLogMgrEntityT            logMgr;

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
   *  Create GTPv1u Stack Instance
   *--------------------------------------------------------------------------*/

  rc = nwSdpInitialize(&hSdp);

  if(rc != NW_SDP_OK)
  {
    NW_LOG(NW_LOG_LEVEL_ERRO, "Failed to create Soft Data Plane instance. Error '%u' occured", rc);
    exit(1);
  }

  NW_LOG(NW_LOG_LEVEL_INFO, "SDP Handle '%X' Creation Successful!", hSdp);
 
  /*---------------------------------------------------------------------------
   * Set up Timer Manager Entity 
   *--------------------------------------------------------------------------*/

  tmrMgr.tmrMgrHandle = 0;
  tmrMgr.tmrStartCallback = nwTimerStart;
  tmrMgr.tmrStopCallback = nwTimerStop;

  rc = nwSdpSetTimerMgrEntity(hSdp, &tmrMgr);
  NW_ASSERT( rc == NW_SDP_OK );

  /*---------------------------------------------------------------------------
   * Set up Log Entity 
   *--------------------------------------------------------------------------*/

  logMgr.logMgrHandle   = (NwSdpLogMgrHandleT) &logObj;
  logMgr.logReqCallback  = nwMiniLogMgrLogRequest;

  rc = nwSdpSetLogMgrEntity(hSdp, &logMgr);
  NW_ASSERT( rc == NW_SDP_OK );
  
  /*---------------------------------------------------------------------------
   * Set up Ulp Entity 
   *--------------------------------------------------------------------------*/

  rc = nwMiniUlpInit(&ulpObj, hSdp);
  NW_ASSERT( rc == NW_SDP_OK );

  ulp.hUlp = (NwSdpUlpHandleT) &ulpObj;
  ulp.ulpReqCallback = nwMiniUlpProcessStackReqCallback;

  rc = nwSdpSetUlpEntity(hSdp, &ulp);
  NW_ASSERT( rc == NW_SDP_OK );


  /*---------------------------------------------------------------------------
   * Set up Udp Entity 
   *--------------------------------------------------------------------------*/

  rc = nwMiniUdpInit(&udpObj, hSdp, (argv[2]));
  NW_ASSERT( rc == NW_SDP_OK );

  udp.hUdp = (NwSdpUdpHandleT) &udpObj;
  udp.udpDataReqCallback = nwMiniUdpDataReq;

  rc = nwSdpSetUdpEntity(hSdp, &udp);
  NW_ASSERT( rc == NW_SDP_OK );
 
  /*---------------------------------------------------------------------------
   * Create a GRE interface at Transport Layer Entity 
   *--------------------------------------------------------------------------*/

  rc = nwMiniUdpCreateGreInterface(&udpObj, argv[2], &hGreInterface);
  NW_ASSERT( rc == NW_SDP_OK );

  /*---------------------------------------------------------------------------
   * Create a GRE service at SDP
   *--------------------------------------------------------------------------*/

  rc = nwSdpCreateGreService(hSdp, &hGreService);

  if(rc != NW_SDP_OK)
  {
    NW_LOG(NW_LOG_LEVEL_ERRO, "Failed to create GRE service instance. Error '%u' occured", rc);
    exit(1);
  }

  NW_LOG(NW_LOG_LEVEL_INFO, "Gre Service Handle '%X' Creation Successful!", hGreService);

  /*---------------------------------------------------------------------------
   * Link GRE SDP service and GRE transport interface
   *--------------------------------------------------------------------------*/

  rc = nwMiniUdpGreInterfaceSetSdpService(hGreInterface, hSdp, hGreService);
  NW_ASSERT( rc == NW_SDP_OK );

  rc = nwSdpSetGreServiceTransportInterface(hSdp, hGreService, hGreInterface, nwMiniUdpGreDataReq);
  NW_ASSERT( rc == NW_SDP_OK );

  /*---------------------------------------------------------------------------
   * Set GTPv1u log level  
   *--------------------------------------------------------------------------*/

  rc = nwSdpSetLogLevel(hSdp, logLevel);

  /*---------------------------------------------------------------------------
   *  Send Create Session Request to GTPv1u Stack Instance
   *--------------------------------------------------------------------------*/

  num_of_connections = atoi(argv[1]);

  while ( num_of_connections-- )
  {
    rc = nwMiniUlpCreateConn(&ulpObj, argv[2], 1234 + num_of_connections, argv[3]);
    NW_ASSERT( rc == NW_SDP_OK );
  }

  /*---------------------------------------------------------------------------
   * Event loop 
   *--------------------------------------------------------------------------*/

  NW_EVT_LOOP();

  NW_LOG(NW_LOG_LEVEL_ERRO, "Exit from eventloop, no events to process!");

  /*---------------------------------------------------------------------------
   *  Send Destroy Session Request to GTPv1u Stack Instance
   *--------------------------------------------------------------------------*/

  rc = nwMiniUlpDestroyConn(&ulpObj);
  NW_ASSERT( rc == NW_SDP_OK );


  /*---------------------------------------------------------------------------
   *  Destroy GTPv1u Stack Instance
   *--------------------------------------------------------------------------*/

  rc = nwSdpFinalize(hSdp);

  if(rc != NW_SDP_OK)
  {
    NW_LOG(NW_LOG_LEVEL_ERRO, "Failed to finalize gtpv1u stack instance. Error '%u' occured", rc);
  }
  else
  {
    NW_LOG(NW_LOG_LEVEL_INFO, "Gtpv1u Stack Handle '%X' Finalize Successful!", hSdp);
  }


  return rc;
}

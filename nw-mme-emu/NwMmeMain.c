/*----------------------------------------------------------------------------*
 *                                                                            *
 *                                n w - m m e                                 * 
 *    L T E / S A E    M O B I L I T Y   M A N A G E M E N T   E N T I T Y    *
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
 * NOT LIMITED TO, PROCUREMENT OF SUmmeTITUTE GOODS OR SERVICES; LOSS OF USE,  *
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY      *
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT        *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF   *
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.          *
 *----------------------------------------------------------------------------*/

/** 
 * @file NwMmeMain.c
 * @brief This main file demostrates usage of nw-gtpv2c library for a LTE/SAE MME. 
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <event.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>


#include "NwLog.h"
#include "NwUtils.h"
#include "NwMmeLog.h"
#include "NwLogMgr.h"
#include "NwTmrMgr.h"
#include "NwGtpv2c.h"
#include "NwMmeUe.h"
#include "NwMmeUlp.h"
#include "NwMmeDpe.h"
#include "NwUdp.h"

/*---------------------------------------------------------------------------
 *                            M M E    C L A S S 
 *--------------------------------------------------------------------------*/

typedef struct
{
  NwU32T                        mmeIpAddr;
  NwU32T                        sgwIpAddr;
  NwU32T                        pgwIpAddr;
  NwU32T                        numOfUe;
  NwU32T                        rps;
  NwU32T                        sessionTimeout;
  NwUdpT                        udp;
  NwMmeUlpT                     ulp;
  NwGtpv2cStackHandleT          hGtpcStack;
  struct {
    NwMmeDpeT                   *pDpe;           /*< Data Plane Entity   */
    NwU32T                      gtpuIpv4Addr;
    NwU8T                       tunNwIfName[128];
  } dataPlane;

} NwMmeT;


/*---------------------------------------------------------------------------
 *              T M R M G R     E N T I T Y    C A L L B A C K
 *--------------------------------------------------------------------------*/

static void NW_TMR_CALLBACK(nwMmeHandleGtpcv2StackTimerTimeout)
{
  NwRcT rc;
  /*
   *  Send Timeout Request to GTPv2c Stack Instance
   */
  rc = nwGtpv2cProcessTimeout(arg);
  NW_ASSERT( NW_OK == rc );

  return;
}

NwRcT nwGtpv2TimerStartIndication( NwGtpv2cTimerMgrHandleT tmrMgrHandle,
    NwU32T timeoutSec,
    NwU32T timeoutUsec,
    NwU32T tmrType,
    void*  timeoutArg,
    NwGtpv2cTimerHandleT* phTmr)
{
  return nwTmrMgrStartTimer(tmrMgrHandle, timeoutSec, timeoutUsec, tmrType, nwMmeHandleGtpcv2StackTimerTimeout, timeoutArg, (NwTimerHandleT*)phTmr);
}

NwRcT nwGtpv2TimerStopIndication( NwGtpv2cTimerMgrHandleT tmrMgrHandle,
    NwGtpv2cTimerHandleT hTmr)
{
  return nwTmrMgrStopTimer(tmrMgrHandle, (NwTimerHandleT)hTmr);
}

/*---------------------------------------------------------------------------
 *             L O G M G R       E N T I T Y      C A L L B A C K 
 *--------------------------------------------------------------------------*/

static
NwRcT nwLog (NwGtpv2cLogMgrHandleT hlogMgr,
    NwU32T logLevel,
    NwCharT* file,
    NwU32T line,
    NwCharT* logStr)
{
  NwLogMgrT* thiz = (NwLogMgrT*) hlogMgr;
  if(thiz->logLevel >= logLevel)
  {
    nwLogMgrLog(&_gLogMgr, "NWGTPCv2 ", logLevel, file, line, logStr);
  }
  return NW_OK;
}

NwRcT
nwMmeCmdLineHelp()
{
  printf("\nSupported command line arguments are:\n");
  printf("\n+---------------------------+---------------+---------------------------------------+");
  printf("\n| ARGUMENT                  | PRESENCE      | DESCRIPTION                           |");
  printf("\n+---------------------------+---------------+---------------------------------------+");
  printf("\n| --mme-ip | -mi            | MANDATORY     | IP address for the MME control plane. |");
  printf("\n| --sgw-ip | -si            | MANDATORY     | IP address of the target SGW.         |");
  printf("\n| --pgw-ip | -pi            | MANDATORY     | IP address of the target PGW.         |");
  printf("\n| --gtpu-ip | -gi           | MANDATORY     | IP address for the MME user plane.    |");
  printf("\n| --num-of-ue | -nu         | OPTIONAL      | Number of UEs to simulate.            |");
  printf("\n| --session-timeout | -st   | OPTIONAL      | UE session timeout in seconds.        |");
  printf("\n| --reg-per-sec| -rps       | OPTIONAL      | Session registrations per second.     |");
  printf("\n| --tun-if  | -si           | OPTIONAL      | Network interface name for tunnel-if. |");
  printf("\n+---------------------------+---------------+---------------------------------------+");
  printf("\n\nExample Usage: \n$ nwLteMmeEmu --mme-ip 10.0.0.1 --sgw-ip 10.0.0.2 --pgw-ip 10.0.0.3 --gtpu-ip 10.0.0.1 --tun-if eth1 -nu 50000 -st 120 -rps 100\n");
  printf("\n");
  exit(0);
}

NwRcT
nwMmeParseCmdLineOpts(NwMmeT*  thiz, int argc, char* argv[])
{
  NwRcT rc = NW_OK;
  int i = 0;

  /* Set default values */
  thiz->rps             = 0xffffffff;
  thiz->numOfUe         = 1;
  thiz->mmeIpAddr       = 0;
  thiz->sgwIpAddr       = 0;
  thiz->pgwIpAddr       = 0;
  thiz->sessionTimeout  = 0;

  strcpy((char*)thiz->dataPlane.tunNwIfName, "");
  i++;
  while( i < argc )
  {
    NW_MME_LOG(NW_LOG_LEVEL_DEBG, "Processing cmdline arg %s", argv[i]);
    if((strcmp("--sgw-ip", argv[i]) == 0)
        || (strcmp(argv[i], "-si") == 0))
    {
      i++;
      if(i >= argc)
        return NW_FAILURE;

      NW_MME_LOG(NW_LOG_LEVEL_DEBG, "sgw ip %s", argv[i]);
      thiz->sgwIpAddr = inet_addr(argv[i]);
    }
    else if((strcmp("--pgw-ip", argv[i]) == 0)
        || (strcmp(argv[i], "-pi") == 0))
    {
      i++;
      if(i >= argc)
        return NW_FAILURE;

      NW_MME_LOG(NW_LOG_LEVEL_DEBG, "pgw ip %s", argv[i]);
      thiz->pgwIpAddr = inet_addr(argv[i]);
    }
    else if((strcmp("--mme-ip", argv[i]) == 0)
        || (strcmp(argv[i], "-mi") == 0))
    {
      i++;
      if(i >= argc)
        return NW_FAILURE;

      NW_MME_LOG(NW_LOG_LEVEL_DEBG, "mme ip %s", argv[i]);
      thiz->mmeIpAddr = inet_addr(argv[i]);
    }
    else if((strcmp("--gtpu-ip", argv[i]) == 0)
        || (strcmp(argv[i], "-gi") == 0))
    {
      i++;
      if(i >= argc)
        return NW_FAILURE;

      NW_MME_LOG(NW_LOG_LEVEL_DEBG, "User Plane IP address %s", argv[i]);
      thiz->dataPlane.gtpuIpv4Addr = ntohl(inet_addr(argv[i]));
    }
    else if((strcmp("--tun-if", argv[i]) == 0)
        || (strcmp(argv[i], "-si") == 0))
    {
      i++;
      if(i >= argc)
        return NW_FAILURE;

      NW_MME_LOG(NW_LOG_LEVEL_DEBG, "TUN network inteface name %s", argv[i]);
      strcpy((char*)thiz->dataPlane.tunNwIfName, (argv[i]));
    }
 
    else if((strcmp("--session-timeout", argv[i]) == 0)
        || (strcmp(argv[i], "-st") == 0))
    {
      i++;
      if(i >= argc)
        return NW_FAILURE;

      NW_MME_LOG(NW_LOG_LEVEL_DEBG, "UE Session timeout %s", argv[i]);
      thiz->sessionTimeout = atoi(argv[i]);
    }
    else if((strcmp("--num-of-ue", argv[i]) == 0)
        || (strcmp(argv[i], "-nu") == 0))
    {
      i++;
      if(i >= argc)
        return NW_FAILURE;

      NW_MME_LOG(NW_LOG_LEVEL_DEBG, "number of UE %s", argv[i]);
      thiz->numOfUe = atoi(argv[i]);
    }
    else if((strcmp("--reg-per-sec", argv[i]) == 0)
        || (strcmp(argv[i], "-rps") == 0))
    {
      i++;
      if(i >= argc)
        return NW_FAILURE;

      NW_MME_LOG(NW_LOG_LEVEL_DEBG, "Registration per second %s", argv[i]);
      thiz->rps = atoi(argv[i]);
    }

    else if((strcmp("--help", argv[i]) == 0)
        || (strcmp(argv[i], "-h") == 0))
    {
      nwMmeCmdLineHelp();
    }
    else
    {
      rc = NW_FAILURE;
    }
    i++;
  }

  if(thiz->mmeIpAddr && thiz->sgwIpAddr && thiz->pgwIpAddr)
  {
    return rc;
  }
  else
  {
    return NW_FAILURE;
  }
}


/*---------------------------------------------------------------------------
 *                T H E      M A I N      F U N C T I O N 
 *--------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
  NwRcT rc; 

  NwMmeT                    mme;
  NwGtpv2cUlpEntityT        ulp;
  NwGtpv2cUdpEntityT        udp;
  NwGtpv2cTimerMgrEntityT   tmrMgr;
  NwGtpv2cLogMgrEntityT     logMgr;


  /*---------------------------------------------------------------------------
   *  Initialize LogMgr
   *--------------------------------------------------------------------------*/

  rc = nwLogMgrInit(nwLogMgrGetInstance(), (NwU8T*)"NW-MME", getpid());
  NW_ASSERT(NW_OK == rc);

  /*---------------------------------------------------------------------------
   *  Parse Commandline Arguments 
   *--------------------------------------------------------------------------*/

  rc = nwMmeParseCmdLineOpts(&mme, argc, argv);
  if(rc != NW_OK)
  {
    printf("Usage error. Please refer help.\n");
    printf("Example usage: %s --sgw-ip <a.b.c.d> --mme-ip <x.y.z.a> --pgw-ip <e.f.g.h> --num-of-ue 10\n", argv[0]);
    exit(0);
  }

  /*---------------------------------------------------------------------------
   *  Initialize event library
   *--------------------------------------------------------------------------*/

  NW_EVT_INIT();

  /*---------------------------------------------------------------------------
   * Create Data Plane instance.
   *--------------------------------------------------------------------------*/

  mme.dataPlane.pDpe    = nwMmeDpeInitialize();

  if(mme.dataPlane.gtpuIpv4Addr)
  {
    rc = nwMmeDpeCreateGtpuService(mme.dataPlane.pDpe, mme.dataPlane.gtpuIpv4Addr);
  }

  if(strlen(mme.dataPlane.tunNwIfName) !=0)
  {
    rc = nwMmeDpeCreateIpv4Service(mme.dataPlane.pDpe, mme.dataPlane.tunNwIfName);
  }

  /*---------------------------------------------------------------------------
   *  Initialize GTPv2c Stack Instance
   *--------------------------------------------------------------------------*/

  rc = nwGtpv2cInitialize(&mme.hGtpcStack);

  if(rc != NW_OK)
  {
    NW_MME_LOG(NW_LOG_LEVEL_ERRO, "Failed to create GTPv2c stack instance. Error '%u' occured", rc);
    exit(1);
  }

  NW_MME_LOG(NW_LOG_LEVEL_INFO, "GTP-Cv2 Stack Handle '%X' Creation Successful!", mme.hGtpcStack);

  /* Set up Log Entity */ 

  logMgr.logMgrHandle   = (NwGtpv2cLogMgrHandleT) nwLogMgrGetInstance();
  logMgr.logReqCallback  = nwLog;

  rc = nwGtpv2cSetLogMgrEntity(mme.hGtpcStack, &logMgr);
  NW_ASSERT( NW_OK == rc );

  rc = nwGtpv2cSetLogLevel(mme.hGtpcStack, nwLogMgrGetLogLevel(nwLogMgrGetInstance()));
  NW_ASSERT( NW_OK == rc );

  /* Set up Timer Entity */

  tmrMgr.tmrMgrHandle   = 0;
  tmrMgr.tmrStartCallback= nwGtpv2TimerStartIndication;
  tmrMgr.tmrStopCallback = nwGtpv2TimerStopIndication;

  rc = nwGtpv2cSetTimerMgrEntity(mme.hGtpcStack, &tmrMgr);
  NW_ASSERT( NW_OK == rc );

  /* Initialize and Set up Ulp Entity */

  rc = nwMmeUlpInit(&mme.ulp, mme.numOfUe, mme.sessionTimeout, mme.mmeIpAddr, mme.sgwIpAddr, mme.pgwIpAddr, mme.rps, mme.dataPlane.pDpe, mme.hGtpcStack);
  NW_ASSERT( NW_OK == rc );

  ulp.hUlp = (NwGtpv2cUlpHandleT) &mme.ulp;
  ulp.ulpReqCallback = nwMmeUlpStackReqCallback;

  rc = nwGtpv2cSetUlpEntity(mme.hGtpcStack, &ulp);
  NW_ASSERT( NW_OK == rc );


  /* Initialize and Set up Udp Entity */

  rc = nwUdpInit(&mme.udp, mme.mmeIpAddr, mme.hGtpcStack);
  NW_ASSERT( NW_OK == rc );

  udp.hUdp              = (NwGtpv2cUdpHandleT)&mme.udp;
  udp.udpDataReqCallback = nwUdpDataReq;
  rc = nwGtpv2cSetUdpEntity(mme.hGtpcStack, &udp);
  NW_ASSERT( NW_OK == rc );


  /*---------------------------------------------------------------------------
   * Start Network Entry Procedure from ULP 
   *--------------------------------------------------------------------------*/

  rc = nwMmeUlpStartNetworkEntry(&mme.ulp);
  NW_ASSERT( NW_OK == rc );


  /*---------------------------------------------------------------------------
   * Event Loop 
   *--------------------------------------------------------------------------*/

  NW_EVT_LOOP();

  NW_MME_LOG(NW_LOG_LEVEL_ERRO, "Exit from eventloop, no events to process!");

  /*---------------------------------------------------------------------------
   *  Destroy ULP instance 
   *--------------------------------------------------------------------------*/

  rc = nwMmeUlpDestroy(&mme.ulp);
  NW_ASSERT( NW_OK == rc );


  /*---------------------------------------------------------------------------
   *  Destroy GTPv2c Stack Instance
   *--------------------------------------------------------------------------*/

  rc = nwGtpv2cFinalize(mme.hGtpcStack);

  if(rc != NW_OK)
  {
    NW_MME_LOG(NW_LOG_LEVEL_ERRO, "Failed to finalize GTPv2c stack instance. Error '%u' occured", rc);
  }
  else
  {
    NW_MME_LOG(NW_LOG_LEVEL_INFO, "GTP-Cv2 Stack Handle '%x' Finalize Successful!", mme.hGtpcStack);
  }

  return rc;
}

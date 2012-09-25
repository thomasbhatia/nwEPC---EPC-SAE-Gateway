/*----------------------------------------------------------------------------*
 *                                                                            *
 *         M I N I M A L I S T I C    T M R M G R     E N T I T Y             *
 *                                                                            *
 *                    Copyright (C) 2010 Amit Chawre.                         *
 *                                                                            *
 *----------------------------------------------------------------------------*/


/** 
 * @file NwMiniTmrMgrEntity.h
 * @brief This file ontains example of a minimalistic timer manager entity.
*/

#include <stdio.h>
#include <assert.h>
#include "NwEvt.h"

#ifndef NW_ASSERT
#define NW_ASSERT assert
#endif 

#ifndef __NW_MINI_TMR_MGR_H__
#define __NW_MINI_TMR_MGR_H__

typedef struct
{
  NwEventT     ev;
  void*  timeoutArg;
} NwMiniTmrT;


#ifdef __cplusplus
extern "C" {
#endif

NwIpv4RcT nwTimerStart( NwIpv4TimerMgrHandleT tmrMgrHandle,
    NwU32T timeoutSec,
    NwU32T timeoutUsec,
    NwU32T tmrType,
    void*  timeoutArg,
    NwIpv4TimerHandleT* hTmr);


NwIpv4RcT nwTimerStop( NwIpv4TimerMgrHandleT tmrMgrHandle,
    NwIpv4TimerHandleT hTmr);


#ifdef __cplusplus
}
#endif

#endif

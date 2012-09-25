/*----------------------------------------------------------------------------*
 *                                                                            *
 *                              n w - e v e n t                               * 
 *                                                                            *
 *     A s y n c h r o n o u s    E v e n t    N o t i f i c a t i o n        *
 *                               L i b r a r y                                *
 *                                                                            *
 *                                                                            *
 * Copyright (c) 2010, Amit Chawre                                            *
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

#ifndef __NW_EVENT_H__
#define __NW_EVENT_H__

#include <sys/time.h>

#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 @mainpage
 Event Notification Library
 */

 /**
  @file NwEvent.h
  */

/*--------------------------------------------------------------------------*
 *          S H A R E D      M A C R O      D E F I N I T I O N S           *
 *--------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * Type Definitions
 *--------------------------------------------------------------------------*/

#define NW_EVENT_IN     /**< An input argument  */
#define NW_EVENT_OUT    /**< An output argumnet */
#define NW_EVENT_INOUT  /**< An input and output argument */

#define NW_EVENT_TRUE                                   (1)             /**< Truth value        */
#define NW_EVENT_FALSE                                  (0)             /**< False value        */

typedef unsigned char                                   NwEventU8T;     /**< Unsigned 1 byte    */
typedef unsigned short                                  NwEventU16T;    /**< Unsigned 2 byte    */
typedef unsigned int                                    NwEventU32T;    /**< Unsigned 4 byte    */
typedef char                                            NwEventCharT;   /**< Strings            */
typedef struct timeval                                  NwTimevalT;     /**< Time value         */


/*---------------------------------------------------------------------------
 * Return Code Definitions
 *--------------------------------------------------------------------------*/

typedef enum {
  NW_EVENT_OK                                           = 0x00000000,   /**< Return code OK     */
  NW_EVENT_FAILURE                                      = 0xFFFFFFFE    /**< Return Code Failure*/
} NwEventRcT;


/*---------------------------------------------------------------------------
 * Log Level Definitions
 *--------------------------------------------------------------------------*/

#define NW_EVENT_LOG_LEVEL_EMER                         (0) /**< System is unusable              */
#define NW_EVENT_LOG_LEVEL_ALER                         (1) /**< Action must be taken immediately*/
#define NW_EVENT_LOG_LEVEL_CRIT                         (2) /**< Critical conditions             */
#define NW_EVENT_LOG_LEVEL_ERRO                         (3) /**< Error conditions                */
#define NW_EVENT_LOG_LEVEL_WARN                         (4) /**< Warning conditions              */ 
#define NW_EVENT_LOG_LEVEL_NOTI                         (5) /**< Normal but signification condition */
#define NW_EVENT_LOG_LEVEL_INFO                         (6) /**< Informational                   */
#define NW_EVENT_LOG_LEVEL_DEBG                         (7) /**< Debug-level messages            */


/*---------------------------------------------------------------------------
 * Assertion macro
 *--------------------------------------------------------------------------*/

#define NW_EVENT_ASSERT                                 assert /**< Assertion */


/*---------------------------------------------------------------------------
 * Timer Type Flags
 *--------------------------------------------------------------------------*/

#define NW_EVENT_TIMER_REPETITIVE                       (0x8000)        /**< A repetitive timer */ 
#define NW_EVENT_TIMER_ONE_SHOT                         (0x4000)        /**< A one shot timer   */

/*---------------------------------------------------------------------------
 * Event Type Flags
 *--------------------------------------------------------------------------*/

#define NW_EVENT_READ                                   (0x0002)        /**< A read event       */
#define NW_EVENT_WRITE                                  (0x0004)        /**< A write event      */

/*---------------------------------------------------------------------------
 * Event Loop Flags
 *--------------------------------------------------------------------------*/

#define NW_EVENT_LOOP_NONBLOCK                          (1)             /**< Execute event loop in non-blocking mode */


/*---------------------------------------------------------------------------
 * Timer and Event callback function
 *--------------------------------------------------------------------------*/

typedef void                                            (*NwEventCallbackT) (void*); /**< imer and Event callback functio */


/*---------------------------------------------------------------------------
 *         E V E N T      C L A S S       D E F I N I T I O N
 *--------------------------------------------------------------------------*/

typedef TAILQ_ENTRY(NwEvent) NwTimerTailqEntryT;
typedef TAILQ_ENTRY(NwEvent) NwEventTailqEntryT;
typedef TAILQ_ENTRY(NwEvent) NwActiveTailqEntryT;

/**
 * Event class definition
 */

typedef struct NwEvent 
{
  NwEventCallbackT              eventCallback;       
  void*                         eventCallbackArg;
  NwEventU32T                   eventPriority;       /* Smaller numbers are higher priority*/
  NwEventU32T                   eventFd;             /* File Descriptor  */
  NwEventU32T                   eventFlags;
  NwTimevalT                    eventTimeout;
  NwTimevalT                    eventDuration;
  NwActiveTailqEntryT           eventActiveqEntry;       
  NwEventU32T                   timerMinHeapIndex;
  void*                         eventBase;
  struct NwEvent                *pNext;
} NwEventT;

typedef NwEventT NwTimerEventT;


/*---------------------------------------------------------------------------
 *                      P U B L I C   M E T H O D S
 *--------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * Constructor
 *--------------------------------------------------------------------------*/

/** 
 Initialize a global instance of library.

 @return NW_OK on success.
 */

extern NwEventRcT 
nwEventInitialize();

/*---------------------------------------------------------------------------
 * Destructor
 *--------------------------------------------------------------------------*/

/** 
 Finalize library.

 @return NW_OK on success.
 */

extern NwEventRcT 
nwEventFinalize();

/** 
 Initialize a event priority levels library.

 @param[in] noOfPriorityLevels: No of priority levels.
 @return NW_OK on success.
 */

extern NwEventRcT
nwEventPriorityInit( NwEventU32T noOfPriorityLevels);


/*---------------------------------------------------------------------------
 * Timer Operations
 *--------------------------------------------------------------------------*/

/**
 Create a timer. 
     
 @param[out] ppTmrId: Pointer to Timer Handle.
 @param[in] cb: The function to be called back on timeout.
 @param[in] cbArg: The argument to the callback on timeout.
 @param[in] timeoutSec: Timeout value in seconds.
 @param[in] timeoutUsec: Timeout value in micro-seconds.
 @param[in] timerFlags: Timer flags.
 @return NW_OK on success.
 */

extern NwEventRcT 
nwEventTimerCreate(NwEventT**           ppTmrId,                /* pointer to the timer handle         */
                   NwEventCallbackT     cb,                     /* function to be called on timeout     */ 
                   void*                cbArg,                  /* argument to the callback on timeout  */
                   NwEventU32T          timeoutSec,             /* timeout, in sec                      */
                   NwEventU32T          timeoutUsec,            /* timeout, in micro sec                */
                   NwEventU32T          timerFlags );           /* one shot or repetitive               */
               

/**
 Create and start a timer. 
     
 @param[out] ppTmrId: Timer Handle.
 @param[in] cb: The function to be called back on timeout.
 @param[in] cbArg: The argument to the callback on timeout.
 @param[in] timeoutSec: Timeout value in seconds.
 @param[in] timeoutUsec: Timeout value in micro-seconds.
 @param[in] timerFlags: Timer flags.
 @return NW_OK on success.
 */

extern NwEventRcT 
nwEventTimerCreateAndStart(NwEventT**           ppTmrId,        /* pointer to the timer handle          */ 
                           NwEventCallbackT     cb,             /* function to be called on timeout     */
                           void*                cbArg,          /* argument to the callback on timeout  */
                           NwEventU32T          timeoutSec,     /* timeout, in sec                      */
                           NwEventU32T          timeoutUsec,    /* timeout, in micro sec                */
                           NwEventU32T          timerFlags  );  /* one shot or repetitive               */


/**
 Destroy a timer. 
     
 @param[out] pTmrId: Timer Handle.
 @return NW_OK on success.
 */

extern NwEventRcT 
nwEventTimerDestroy (NwEventT*  pTmrId);

/**
 Start a timer. 
     
 @param[out] pTmrId: Timer Handle.
 @return NW_OK on success.
 */

extern NwEventRcT 
nwEventTimerStart(NwEventT*     pTmrId);              


/**
 Stop a timer. 
     
 @param[out] pTmrId: Timer Handle.
 @return NW_OK on success.
 */

extern NwEventRcT 
nwEventTimerStop (NwEventT*     pTmrId);

/**
 Restart a timer. 
     
 @param[out] pTmrId: Timer Handle.
 @return NW_OK on success.
 */

extern NwEventRcT
nwEventTimerRestart ( NwEventT* pTmrId ); 

/**
 Update a timer. 
     
 @param[in] pTmrId: Timer Handle.
 @param[in] tv: New timeout value.
 @return NW_OK on success.
 */

extern NwEventRcT 
nwEventTimerUpdate( NwEventT    *pTmrId,      
                    struct      timeval tv );


/*---------------------------------------------------------------------------
 * Event Operations
 *--------------------------------------------------------------------------*/

/**
 Add a file descriptor event. 
     
 @return NW_OK on success.
 */

extern NwEventRcT 
nwEventAdd(NwEventT             *phEvent,       /**< pointer to event handle                                    */
           NwEventU32T          eventFd,        /**< file descriptor                                            */
           NwEventCallbackT     cb,             /**< function to be called back on the occurence of an event    */
           void                 *cbArg,         /**< argument to the callback function                          */
           NwEventU16T          flags);         /**< event type i.e read or write                               */


/**
 Add a file descriptor event. 
     
 @return NW_OK on success.
 */

extern NwEventRcT 
nwEventRemove(NwEventT *phEvent                 /**< pointer to event handle                                    */);

/*---------------------------------------------------------------------------
 * Event Loop Operations
 *--------------------------------------------------------------------------*/

/**
 Execute the Event Loop. 
     
 @return NW_OK on success.
 */

extern NwEventRcT 
nwEventLoop();


/**
 Execute the Event Loop Once. 
     
 @return NW_OK on success.
 */

extern NwEventRcT 
nwEventLoopOnce(NwEventU32T flags);

#ifdef __cplusplus
}
#endif

#endif

/*--------------------------------------------------------------------------*
 *                      E N D     O F    F I L E                            *
 *--------------------------------------------------------------------------*/


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


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>
#include <sys/time.h>
#include <sys/select.h>

#include "NwEvent.h"

#define __MIN_HEAP_SIZE__                                               (100000)

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------*
 *                    M A C R O    D E F I N I T I O N S                    *
 *--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*
 * Logging Related Macros                                                   *
 *--------------------------------------------------------------------------*/

#ifdef __WITH_LOGS__

static
NwEventCharT* gLogLevelStr[] = {"EMER", "ALER", "CRIT",  "ERRO", "WARN", "NOTI", "INFO", "DEBG"};

#define NW_LOG(_logLevel, ...)                                                  \
  do {                                                                          \
    char _logStr[1024];                                                         \
    snprintf(_logStr, 1024, __VA_ARGS__);                                       \
    printf(" NWEVENT     %s - %s <%s,%u>\n", gLogLevelStr[_logLevel], _logStr, (__FILE__), __LINE__);\
  } while(0)


#define NW_EMER(...)                    NW_LOG(NW_EVENT_LOG_LEVEL_EMER, __VA_ARGS__)
#define NW_ALER(...)                    NW_LOG(NW_EVENT_LOG_LEVEL_ALER, __VA_ARGS__)
#define NW_CRIT(...)                    NW_LOG(NW_EVENT_LOG_LEVEL_CRIT, __VA_ARGS__)
#define NW_ERRO(...)                    NW_LOG(NW_EVENT_LOG_LEVEL_ERRO, __VA_ARGS__)
#define NW_WARN(...)                    NW_LOG(NW_EVENT_LOG_LEVEL_WARN, __VA_ARGS__)
#define NW_INF0(...)                    NW_LOG(NW_EVENT_LOG_LEVEL_INFO, __VA_ARGS__)
#define NW_DEBG(...)                    NW_LOG(NW_EVENT_LOG_LEVEL_DEBG, __VA_ARGS__)

#define NW_ENTER()                      NW_LOG(NW_EVENT_LOG_LEVEL_DEBG, "Entering %s", __func__)
#define NW_LEAVE()                      NW_LOG(NW_EVENT_LOG_LEVEL_DEBG, "Leaving  %s", __func__)

#else

#define NW_EMER(...)                    
#define NW_ALER(...)                    
#define NW_CRIT(...)                   
#define NW_ERRO(...)                  
#define NW_WARN(...)                   
#define NW_INF0(...)                  
#define NW_DEBG(...)                 

#define NW_ENTER()                      
#define NW_LEAVE()                      

#endif  /* __WITH_LOGS__ */

#define NW_EVENT_ACTIVE                                                 (0x0080) 

/*--------------------------------------------------------------------------*
 * Timer Related Macros                                                     *
 *--------------------------------------------------------------------------*/

#ifdef _EVENT_HAVE_TIMERADD
#define NW_EVENT_TIMER_ADD(tvp, uvp, vvp) timeradd((tvp), (uvp), (vvp))
#define NW_EVENT_TIMER_SUB(tvp, uvp, vvp) timersub((tvp), (uvp), (vvp))
#else

#define NW_EVENT_TIMER_ADD(tvp, uvp, vvp)				\
  do {									\
    (vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec;			\
    (vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec;                   \
    if ((vvp)->tv_usec >= 1000000) {					\
      (vvp)->tv_sec++;							\
      (vvp)->tv_usec -= 1000000;					\
    }									\
  } while (0)

#define	NW_EVENT_TIMER_SUB(tvp, uvp, vvp)				\
  do {									\
    (vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec;      		\
    (vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec;	                \
    if ((vvp)->tv_usec < 0) {						\
      (vvp)->tv_sec--;							\
      (vvp)->tv_usec += 1000000;					\
    }									\
  } while (0)

#endif /* _EVENT_HAVE_TIMERADD */

#ifdef _EVENT_HAVE_TIMERCLEAR
#define NW_EVENT_TIMER_CLEAR(tvp)      timerclear(tvp)
#else
#define	NW_EVENT_TIMER_CLEAR(tvp)	(tvp)->tv_sec = (tvp)->tv_usec = 0
#endif

#define	NW_EVENT_TIMER_CMP(tvp, uvp, cmp)				\
	(((tvp).tv_sec == (uvp).tv_sec) ?				\
	 ((tvp).tv_usec cmp (uvp).tv_usec) :				\
	 ((tvp).tv_sec cmp (uvp).tv_sec))




typedef struct
{
  int currSize;
  int maxSize;
  NwTimerEventT** pHeap;
} NwTimerMinHeapT;


/*--------------------------------------------------------------------------*
 * Singleton event engine instance
 *--------------------------------------------------------------------------*/

#define NW_EVENT_MAX_FILE_DESCRIPTORS                           (1024)
typedef struct 
{
  NwEventU8T                    isInitialized;
  NwEventU32T                   fdCount;
  NwEventU32T                   highestFd; 
  NwEventU32T                   tmrCount;
  NwEventU32T                   tmrCountMax;
  NwEventU32T                   numActiveQueues;
  fd_set                        eventReadSet;
  fd_set                        eventWriteSet;
  NwEventT*                     eventReadFdMap[NW_EVENT_MAX_FILE_DESCRIPTORS];
  NwEventT*                     eventWriteFdMap[NW_EVENT_MAX_FILE_DESCRIPTORS];
  NwTimerMinHeapT*              timerMinHeap;

  TAILQ_HEAD (NwEventQueue, NwEvent) eventQueue;
  TAILQ_HEAD(NwEventList, NwEvent)** ppActiveQueues;
} NwEventEngineT;


static NwEventEngineT           gInstance = { 0,};
static NwEventT*                gpEventPool = NULL;

/*--------------------------------------------------------------------------*
 *                    P R I V A T E   F U N C T I O N S                     *
 *--------------------------------------------------------------------------*/

#define NW_HEAP_PARENT_INDEX(__child)           ( ( (__child) - 1 ) / 2 )
#define NW_MIN_HEAP_INDEX_INVALID               (0xFFFFFFFF)

NwTimerMinHeapT*
nwEventTimerMinHeapNew(int maxSize)
{
  NwTimerMinHeapT* thiz = (NwTimerMinHeapT*) malloc (sizeof(NwTimerMinHeapT));
  if(thiz)
  {
    thiz->currSize = 0;
    thiz->maxSize = maxSize;
    thiz->pHeap = (NwTimerEventT**) malloc (maxSize * sizeof(NwTimerEventT*));
  }
  return thiz;
}

void
nwEventTimerMinHeapDelete(NwTimerMinHeapT* thiz)
{
  free(thiz->pHeap);
  free(thiz);
}

static NwEventRcT
nwEventTimerMinHeapInsert(NwTimerMinHeapT* thiz, NwTimerEventT *pTimerEvent)
{
  int holeIndex = thiz->currSize++;

  while((holeIndex > 0) && 
      NW_EVENT_TIMER_CMP((thiz->pHeap[NW_HEAP_PARENT_INDEX(holeIndex)])->eventTimeout, pTimerEvent->eventTimeout, >))
  {
    thiz->pHeap[holeIndex] = thiz->pHeap[NW_HEAP_PARENT_INDEX(holeIndex)];
    thiz->pHeap[holeIndex]->timerMinHeapIndex = holeIndex;
    holeIndex = NW_HEAP_PARENT_INDEX(holeIndex);
  }

  thiz->pHeap[holeIndex] = pTimerEvent;
  pTimerEvent->timerMinHeapIndex = holeIndex;

  return holeIndex;
}

static NwEventRcT
nwEventTimerMinHeapRemove(NwTimerMinHeapT* thiz, int minHeapIndex)
{
  NwTimerEventT* pTimerEvent;
  int holeIndex = minHeapIndex;
  int minChild, maxChild;

  if(minHeapIndex == NW_MIN_HEAP_INDEX_INVALID) return NW_EVENT_FAILURE;
  if(minHeapIndex < thiz->currSize)
  {
    thiz->pHeap[minHeapIndex]->timerMinHeapIndex = NW_MIN_HEAP_INDEX_INVALID;
    thiz->currSize--;

    pTimerEvent = thiz->pHeap[thiz->currSize];
    holeIndex = minHeapIndex;
    minChild = ( 2 * holeIndex ) + 1;
    maxChild = minChild + 1;

    while( (maxChild) <= thiz->currSize )
    {
      if(NW_EVENT_TIMER_CMP(thiz->pHeap[minChild]->eventTimeout, thiz->pHeap[maxChild]->eventTimeout, >))
        minChild = maxChild; 

      if(NW_EVENT_TIMER_CMP(pTimerEvent->eventTimeout, thiz->pHeap[minChild]->eventTimeout, <))
      {
        break;
      }
      thiz->pHeap[holeIndex] = thiz->pHeap[minChild];
      thiz->pHeap[holeIndex]->timerMinHeapIndex = holeIndex;
      holeIndex = minChild;
      minChild = ( 2 * holeIndex ) + 1;
      maxChild = minChild + 1;
    }

    while((holeIndex > 0) &&
        NW_EVENT_TIMER_CMP((thiz->pHeap[NW_HEAP_PARENT_INDEX(holeIndex)])->eventTimeout, pTimerEvent->eventTimeout, >))
    {
      thiz->pHeap[holeIndex] = thiz->pHeap[NW_HEAP_PARENT_INDEX(holeIndex)];
      thiz->pHeap[holeIndex]->timerMinHeapIndex = holeIndex;
      holeIndex = NW_HEAP_PARENT_INDEX(holeIndex);
    }


    if(holeIndex < thiz->currSize)
    {
      thiz->pHeap[holeIndex] = pTimerEvent;
      pTimerEvent->timerMinHeapIndex = holeIndex;
    }

    thiz->pHeap[thiz->currSize] = NULL;
    return NW_EVENT_OK;
  }
  return NW_EVENT_FAILURE;
}

static inline NwTimerEventT* 
nwEventTimerMinHeapPeek(NwTimerMinHeapT* thiz)
{
  if(thiz->currSize)
  {
    return thiz->pHeap[0];
  }
  return NULL;
}

static NwEventRcT
nwEventGetNextTimeout(struct timeval **tv_p)
{
  struct timeval now;
  NwTimerEventT *ev;
  struct timeval *tv = *tv_p;

  NW_ENTER();

  if ((ev = nwEventTimerMinHeapPeek(gInstance.timerMinHeap)) == NULL) {
    /* if no time-based events are active wait for I/O */
    *tv_p = NULL;
    NW_LEAVE();
    return (0);
  }
  if (gettimeofday(&now, NULL) == -1)
  {
    NW_LEAVE();
    return (-1);
  }

  if (NW_EVENT_TIMER_CMP((ev->eventTimeout), now, <=)) {
    (tv)->tv_sec = (tv)->tv_usec = 0; 
    NW_LEAVE();
    return (0);
  }

  if(((ev->eventTimeout.tv_usec - now.tv_usec) / 1000000) >= 1) 
  {
    (tv)->tv_sec  = (ev->eventTimeout.tv_sec - now.tv_sec) + ((ev->eventTimeout.tv_usec - now.tv_usec) / 1000000);
    (tv)->tv_usec = (ev->eventTimeout.tv_usec - now.tv_usec) %  1000000 ;
  }
  else
  {
    NW_EVENT_TIMER_SUB(&ev->eventTimeout, &now, tv);
  }

  NW_LEAVE();
  return (0);
}

/*--------------------------------------------------------------------------*
 * Event Priority Related Opertions
 *--------------------------------------------------------------------------*/

static NwEventRcT
nwEventSetNoOfPriorities(NwEventU32T noOfPriorities)
{
  NwEventRcT rc = NW_EVENT_OK;
  NW_ENTER();
  gInstance.numActiveQueues = noOfPriorities;
  NW_LEAVE();
  return rc;
}

static NwEventRcT
nwEventCreateActiveQueues()
{
  NwEventRcT rc = NW_EVENT_OK;
  NwEventU32T i;
  NW_ENTER();
  gInstance.ppActiveQueues = (struct NwEventList **)calloc(gInstance.numActiveQueues,
      gInstance.numActiveQueues * sizeof(struct NwEventList *));
  if (gInstance.ppActiveQueues == NULL)
    NW_ERRO("Error allocating memory");
  else
  {
    for (i = 0; i < gInstance.numActiveQueues; ++i)
    {
      gInstance.ppActiveQueues[i] = (struct NwEventList*)malloc(sizeof(struct NwEventList));
      if (gInstance.ppActiveQueues[i] == NULL)
        NW_ERRO("Error allocating memory");
      TAILQ_INIT(gInstance.ppActiveQueues[i]);
    }
  }
  NW_LEAVE();
  return rc;
}

static NwEventRcT
nwEventInsertIntoActiveQueues(NwEventT *pEvent)
{
  NwEventRcT rc = NW_EVENT_OK;
  NW_ENTER();
  if( pEvent->eventPriority > gInstance.numActiveQueues-1)
  {
    NW_ERRO("##### Priority ##### %d",pEvent->eventPriority);
  }
  else
  {
    NW_DEBG("Adding event %p to active queue %d", pEvent, pEvent->eventPriority);
    TAILQ_INSERT_TAIL(gInstance.ppActiveQueues[pEvent->eventPriority], pEvent, eventActiveqEntry/*ev_entries*/);
    pEvent->eventFlags |= NW_EVENT_ACTIVE;
  }

  NW_LEAVE();
  return rc; 
} 

static NwEventRcT
nwEventRemoveFromActiveQueue(NwEventT *pEvent)
{
  NwEventRcT            rc = NW_EVENT_FAILURE;
  NwEventT            *p_iterator;
  struct NwEventList    *activeq_head = gInstance.ppActiveQueues[pEvent->eventPriority];
  NW_WARN("Removing event %p from ActivQ %u (%p) ", pEvent, pEvent->eventPriority, activeq_head);
  p_iterator = TAILQ_FIRST(activeq_head);
  for ((p_iterator) = TAILQ_FIRST((activeq_head));                       
      (p_iterator);                                                      
      (p_iterator) = TAILQ_NEXT((p_iterator), eventActiveqEntry))
  {
    NW_DEBG("Iterator %p pevent %p", p_iterator, pEvent);
    if(p_iterator == pEvent)
    {
      TAILQ_REMOVE(activeq_head, pEvent, eventActiveqEntry);
      pEvent->eventFlags &= ~(pEvent->eventFlags & NW_EVENT_ACTIVE);
      rc = NW_EVENT_OK;
      break;
    }
  }
  return rc;
}

/*--------------------------------------------------------------------------*
 * Event Processing Opertions
 *--------------------------------------------------------------------------*/

static NwEventRcT
nwEventDispatch()
{
  NwEventRcT            rc      = NW_EVENT_OK;
  NwEventT              *pEvent = NULL;
  struct NwEventList    *activeq= NULL;
  NwEventU32T           i       = 0;
  NwEventCallbackT      eventCallback;       
  void*                 eventCallbackArg;

  NW_ENTER();

  for (i = 0; i < gInstance.numActiveQueues; i++)
  {
    activeq = gInstance.ppActiveQueues[i];
    if (TAILQ_FIRST(gInstance.ppActiveQueues[i]) != NULL) 
    {
      if(activeq != NULL)
      {
        for (pEvent = TAILQ_FIRST(activeq); !TAILQ_EMPTY(activeq) ; pEvent = TAILQ_FIRST(activeq))
        {
          TAILQ_REMOVE(activeq, pEvent, eventActiveqEntry);
          if (pEvent->eventBase != &gInstance)
          {

            NW_CRIT("Illegal event %p with Event Priority %u, QPointer %p Flags = %X",pEvent, pEvent->eventPriority, gInstance.ppActiveQueues[(unsigned int)pEvent->eventPriority], (pEvent->eventFlags & NW_EVENT_ACTIVE));
            continue;
          }

          eventCallback     = pEvent->eventCallback;
          eventCallbackArg  = pEvent->eventCallbackArg;

          pEvent->eventFlags &= ~(pEvent->eventFlags & NW_EVENT_ACTIVE);

          if(pEvent->eventFlags & NW_EVENT_TIMER_REPETITIVE)
          {
            NW_DEBG("Adding repetitive timer %p ack to min heap.", pEvent);
            NW_EVENT_TIMER_ADD(&pEvent->eventTimeout, &pEvent->eventDuration, &pEvent->eventTimeout);
            rc = nwEventTimerMinHeapInsert(gInstance.timerMinHeap, pEvent);
          }
          else if(pEvent->eventFlags & NW_EVENT_TIMER_ONE_SHOT)
          {
            nwEventTimerDestroy (pEvent);
          }

          eventCallback(eventCallbackArg);
        }
      }
      else
      {
        NW_DEBG("No active events in %d active queue", i);
      } 
    }
    else
    {
      NW_DEBG("No events in %d active queue", i);
    } 
  }

  NW_LEAVE();
  return rc;
} 

static NwEventRcT
nwEvProcessTimeout()
{
  NwEventRcT rc = NW_EVENT_OK;
  struct timeval now;
  NwTimerEventT *pTimerEvent = NULL;

  NW_ENTER();

  while ((pTimerEvent = nwEventTimerMinHeapPeek(gInstance.timerMinHeap))) 
  {
    if (gettimeofday(&now, NULL) == -1)
    {
      NW_LEAVE();
      return rc;
    }

    if (NW_EVENT_TIMER_CMP(pTimerEvent->eventTimeout, now, >))
      break;

    rc = nwEventInsertIntoActiveQueues(pTimerEvent); 

    rc = nwEventTimerMinHeapRemove(gInstance.timerMinHeap, pTimerEvent->timerMinHeapIndex);
  }
  NW_LEAVE();
  return rc;
}


/*--------------------------------------------------------------------------*
 *                     P U B L I C   F U N C T I O N S                      *
 *--------------------------------------------------------------------------*/
    
/*---------------------------------------------------------------------------
 *  Constructor
 *--------------------------------------------------------------------------*/

/** 
 Initialize the library.

 @param[in,out] phWmxAsncpHandle : Pointer to stack handle
 */

NwEventRcT 
nwEventInitialize()
{  
  NwEventRcT rc = NW_EVENT_OK;
  NW_ENTER();

  NW_EVENT_ASSERT(gInstance.isInitialized != NW_EVENT_TRUE);

  memset(&gInstance, 0, sizeof(NwEventEngineT));

  NW_DEBG("Using select for asynchronous timer and file descriptor events.");

  FD_ZERO(&gInstance.eventReadSet);
  FD_ZERO(&gInstance.eventWriteSet);

  gInstance.timerMinHeap = nwEventTimerMinHeapNew(__MIN_HEAP_SIZE__);

  TAILQ_INIT(&gInstance.eventQueue);
  nwEventSetNoOfPriorities(1);
  nwEventCreateActiveQueues();

  gInstance.tmrCount = 0;
  gInstance.isInitialized = NW_EVENT_TRUE;

  NW_LEAVE();
  return rc;
}

/*---------------------------------------------------------------------------
 *  Constructor
 *--------------------------------------------------------------------------*/

/** 
 Finalize the library.

 @param[in,out] phWmxAsncpHandle : Pointer to stack handle
 */

NwEventRcT 
nwEventFinalize()
{
  NwEventRcT rc = NW_EVENT_OK;
  NW_ENTER();
  NW_EVENT_ASSERT(gInstance.isInitialized == NW_EVENT_TRUE);
  NW_LEAVE();
  return rc;
}

/*---------------------------------------------------------------------------
 * Initialize event Priority Levels 
 *--------------------------------------------------------------------------*/

/** 
 Initialize the Event Priority Queues.

 @param[in,out] phWmxAsncpHandle : Pointer to stack handle
 */

NwEventRcT 
nwEventPriorityInit(NwEventU32T noOfPriorities)
{
  NwEventRcT rc = NW_EVENT_OK;
  NwEventU32T i;
  NW_ENTER();
  
  for (i = 0; i < gInstance.numActiveQueues; ++i)
        free(gInstance.ppActiveQueues[i]);
  free(gInstance.ppActiveQueues);

  nwEventSetNoOfPriorities(noOfPriorities);
  nwEventCreateActiveQueues();

  NW_LEAVE();
  return rc;
}

/*---------------------------------------------------------------------------
 * Add a file descriptor evetn to watch 
 *--------------------------------------------------------------------------*/

NwEventRcT
nwEventAdd(NwEventT *pEvent, NwEventU32T fd, NwEventCallbackT cb, void *cbArg, NwEventU16T flags)
{
  NwEventRcT rc       = NW_EVENT_OK;

  NW_ENTER();

  pEvent->eventCallback          = (cb);
  pEvent->eventCallbackArg      = (cbArg);
  pEvent->eventFlags             = flags;
  pEvent->eventFd                = fd;
  pEvent->eventPriority          = 0;
  pEvent->eventBase              = &gInstance;

  NW_DEBG("Adding event %p with fd %d for %s", pEvent, fd, (pEvent->eventFlags  & NW_EVENT_READ? "READ":"WRITE"));

  if(pEvent->eventFlags & NW_EVENT_READ)
  {
    FD_SET(pEvent->eventFd, &gInstance.eventReadSet);
    gInstance.eventReadFdMap[pEvent->eventFd] = pEvent ;
  }
  if(pEvent->eventFlags & NW_EVENT_WRITE)
  {
    FD_SET(pEvent->eventFd, &gInstance.eventWriteSet);
    gInstance.eventWriteFdMap[pEvent->eventFd] = pEvent ;
  }

  if(fd > gInstance.highestFd)
    gInstance.highestFd = fd;

  gInstance.fdCount++;

  NW_LEAVE();
  return rc; 
}
 
/*---------------------------------------------------------------------------
 * Remove a file descriptor event 
 *--------------------------------------------------------------------------*/

NwEventRcT
nwEventRemove( NwEventT  *pEvent )
{
  NwEventRcT rc = NW_EVENT_OK;
  NW_ENTER();
  if (pEvent->eventBase != &gInstance)
  {
    NW_WARN("event %p fd %d non existent.", pEvent, pEvent->eventFd);
    return rc;
  }

  NW_DEBG("Deleting event %p with fd %d for %s", pEvent, pEvent->eventFd, (pEvent->eventFlags & NW_EVENT_READ? "READ":"WRITE"));

  if(pEvent->eventFlags & NW_EVENT_READ)
  {
    NW_DEBG("Deleting event %p with fd %d for READ", pEvent, pEvent->eventFd);
    FD_CLR(pEvent->eventFd, &gInstance.eventReadSet);
    gInstance.eventReadFdMap[pEvent->eventFd] = NULL ;
  }
  if(pEvent->eventFlags & NW_EVENT_WRITE)
  {
    NW_DEBG("Deleting event %p with fd %d for WRITE", pEvent, pEvent->eventFd);
    FD_CLR(pEvent->eventFd, &gInstance.eventWriteSet);
    gInstance.eventWriteFdMap[pEvent->eventFd] = NULL ;
  }

  if(pEvent->eventFlags & NW_EVENT_ACTIVE)
    nwEventRemoveFromActiveQueue(pEvent);

  gInstance.fdCount--;

  NW_LEAVE();
  return rc;
}

/*---------------------------------------------------------------------------
 * Create a timer 
 *--------------------------------------------------------------------------*/

NwEventRcT 
nwEventTimerCreate(NwEventT **ppTimerEvent, NwEventCallbackT cb, void *cbArg, NwEventU32T timeoutSec, NwEventU32T timeoutUsec, NwEventU32T eventFlags)
{
  NwEventRcT rc = NW_EVENT_OK;
  NwEventT *pTimerEvent;

  NW_ENTER();

  if(gpEventPool)
  {
    pTimerEvent = gpEventPool;
    gpEventPool = gpEventPool->pNext;
  }
  else
  {
    pTimerEvent = (NwEventT*) malloc (sizeof(NwEventT));
    gInstance.tmrCount++;
    NW_EVENT_ASSERT(gInstance.tmrCount < __MIN_HEAP_SIZE__);
  }

  NW_DEBG("Creating timer %p of type %s", pTimerEvent, ((eventFlags & NW_EVENT_TIMER_ONE_SHOT)?"NW_EVENT_TIMER_ONE_SHOT": "NW_EVENT_TIMER_REPETITIVE"));


  pTimerEvent->eventCallback            = cb;
  pTimerEvent->eventCallbackArg         = cbArg;
  pTimerEvent->eventFd                  = (NwEventU32T)pTimerEvent;
  pTimerEvent->eventFlags               = eventFlags;
  pTimerEvent->eventDuration.tv_sec     = timeoutSec;
  pTimerEvent->eventDuration.tv_usec    = timeoutUsec;
  pTimerEvent->eventPriority            = 0;
  pTimerEvent->eventBase                = &gInstance;
  pTimerEvent->timerMinHeapIndex        = NW_MIN_HEAP_INDEX_INVALID;


  *ppTimerEvent = pTimerEvent;

  NW_LEAVE();
  return rc;
} 


/*---------------------------------------------------------------------------
 * Start a timer 
 *--------------------------------------------------------------------------*/

NwEventRcT
nwEventTimerStart(NwEventT *pTimerEvent)
{
  NwEventRcT rc = NW_EVENT_OK;
  struct timeval now;
  NW_ENTER();

  NW_DEBG("Starting timer %p", pTimerEvent);

  if (gettimeofday(&now, NULL) == -1)
  {
    NW_LEAVE();
    NW_ERRO("Could not get current time");
    return (-1);
  }          

  NW_EVENT_TIMER_ADD(&now, &pTimerEvent->eventDuration, &pTimerEvent->eventTimeout);

  rc = nwEventTimerMinHeapInsert(gInstance.timerMinHeap, pTimerEvent);

  NW_LEAVE();
  return rc;
}

/*---------------------------------------------------------------------------
 * Create and Start a timer 
 *--------------------------------------------------------------------------*/

NwEventRcT
nwEventTimerCreateAndStart(NwEventT** ppTimerEvent, NwEventCallbackT cb, void *cbArg, NwEventU32T timeoutSec, NwEventU32T timeoutUsec, NwEventU32T eventFlags)
{
  NwEventRcT rc = NW_EVENT_OK;
  NW_ENTER();
  rc = nwEventTimerCreate(ppTimerEvent, cb, cbArg, timeoutSec, timeoutUsec, eventFlags);
  if(rc)
    NW_ERRO("Could not Create Timer");
  rc = nwEventTimerStart(*ppTimerEvent);
  if(rc)
    NW_ERRO("Could not Start Timer");
  NW_LEAVE();
  return rc;
}

/*---------------------------------------------------------------------------
 * Stop a timer 
 *--------------------------------------------------------------------------*/

NwEventRcT
nwEventTimerStop (NwEventT* pTimerEvent)
{
  NwEventRcT rc = NW_EVENT_OK;
  NW_ENTER();
  NW_DEBG("Stopping timer %p", pTimerEvent);

  if(pTimerEvent->eventFlags & NW_EVENT_ACTIVE)
    nwEventRemoveFromActiveQueue(pTimerEvent);

  rc = nwEventTimerMinHeapRemove(gInstance.timerMinHeap, pTimerEvent->timerMinHeapIndex);
  NW_LEAVE();
  return rc;
}

/*---------------------------------------------------------------------------
 * Restart a timer 
 *--------------------------------------------------------------------------*/

 NwEventRcT
nwEventTimerRestart ( NwEventT         *pTimerEvent ) 
{
  NwEventRcT rc = NW_EVENT_OK;
  struct timeval now;
  NW_ENTER();

  NW_DEBG("Restarting timer %p", pTimerEvent);
  rc = nwEventTimerMinHeapRemove(gInstance.timerMinHeap, pTimerEvent->timerMinHeapIndex);

  if (gettimeofday(&now, NULL) == -1)
  {
    NW_LEAVE();
    NW_ERRO("Could not get current time");
    return (-1);
  }          

  NW_EVENT_TIMER_ADD(&now, &pTimerEvent->eventDuration, &pTimerEvent->eventTimeout);

  rc = nwEventTimerMinHeapInsert(gInstance.timerMinHeap, pTimerEvent);

  NW_LEAVE();
  return rc;
}

/*---------------------------------------------------------------------------
 * Destroy a timer 
 *--------------------------------------------------------------------------*/

NwEventRcT
nwEventTimerDestroy (NwEventT*  pTimerEvent )
{
  NwEventRcT rc = NW_EVENT_OK;
  NW_ENTER();

  if (pTimerEvent->eventBase != &gInstance)
  {
    NW_WARN("Timer %p with non existent", pTimerEvent);
    return NW_EVENT_FAILURE;
  }

  NW_DEBG("Destroying timer %p", pTimerEvent);

  if(pTimerEvent->eventFlags & NW_EVENT_ACTIVE)
    nwEventRemoveFromActiveQueue(pTimerEvent);

  rc = nwEventTimerMinHeapRemove(gInstance.timerMinHeap, pTimerEvent->timerMinHeapIndex);

  pTimerEvent->pNext = gpEventPool;
  gpEventPool = pTimerEvent;

  NW_LEAVE();
  return rc;
}

/*---------------------------------------------------------------------------
 * Update a timer 
 *--------------------------------------------------------------------------*/

NwEventRcT
nwEventTimerUpdate(NwEventT *pTimerEvent, struct timeval tv)
{
  NwEventRcT rc = NW_EVENT_OK;
  struct timeval now;
  NW_ENTER();
  NW_DEBG("Updating timer %p", pTimerEvent);

  rc = nwEventTimerMinHeapRemove(gInstance.timerMinHeap, pTimerEvent->timerMinHeapIndex);
  pTimerEvent->eventDuration.tv_sec  = tv.tv_sec;
  pTimerEvent->eventDuration.tv_usec = tv.tv_usec;
  gettimeofday(&now, NULL);
  NW_EVENT_TIMER_ADD(&now, &pTimerEvent->eventDuration, &pTimerEvent->eventTimeout);

  rc = nwEventTimerMinHeapInsert(gInstance.timerMinHeap, pTimerEvent);

  NW_LEAVE();
  return rc;
}


/*---------------------------------------------------------------------------
 * Main Event Loop
 *--------------------------------------------------------------------------*/

NwEventRcT 
nwEventLoop()
{
  NwEventRcT          rc      = NW_EVENT_OK;
  NwEventT            *pEvent   = NULL;
  register NwEventU32T     i       = 0;
  fd_set                rfds;
  fd_set                wfds;
  struct timeval        tv = {0, 0}, 
                        *pTimeval = NULL;

  NW_ENTER();

  do {
    tv.tv_sec =  0;
    tv.tv_usec = 0;

    pTimeval = &tv;

    memcpy(&rfds, &gInstance.eventReadSet, sizeof(fd_set));
    memcpy(&wfds, &gInstance.eventWriteSet, sizeof(fd_set));

    nwEventGetNextTimeout(&pTimeval); 

    if(pTimeval == NULL)
    {
      NW_DEBG("No timers to serve.");
    }
    else
    {
      NW_DEBG("Next timeout in %u sec %u usec", (unsigned int)pTimeval->tv_sec, (unsigned int)pTimeval->tv_usec);
    }

    rc = select(gInstance.highestFd + 1, &rfds, &wfds, NULL, pTimeval);

    nwEvProcessTimeout(); 

/*---------------------------------------------------------------------------
 * A value of 0 indicates that the call timed out and no file
 * descriptors have been selected. 
 *--------------------------------------------------------------------------*/


    if(rc > 0)
    {
      NW_DEBG("Num of Fds to dispatch %d", rc);
      for(i = 0; i <= gInstance.highestFd ; i++)
      {
        pEvent = NULL;

        if (FD_ISSET(i, &rfds)) {
          pEvent = gInstance.eventReadFdMap[i];
        }

        if (FD_ISSET(i, &wfds)) {
          pEvent = gInstance.eventWriteFdMap[i];
        }

        if(pEvent)
          nwEventInsertIntoActiveQueues(pEvent);
      }
    }
    else if (rc < 0)
    {
      NW_CRIT("Select error : %s(%u)", strerror(errno), errno);
    }

    nwEventDispatch();

  } while(1);

  NW_LEAVE();
  return rc;
}

/*---------------------------------------------------------------------------
 * Single Event Loop
 *--------------------------------------------------------------------------*/

NwEventRcT 
nwEventLoopOnce( NwEventU32T flags )
{
  NwEventRcT            rc      = NW_EVENT_OK;
  NwEventT              *pEvent = NULL;
  register NwEventU32T  i    = 0;
  struct timeval        tv = {0, 0}, 
                        *pTimeval = NULL;
  fd_set                rfds;
  fd_set                wfds;

  NW_ENTER();

  do {
    pTimeval = &tv;

    memcpy(&rfds, &gInstance.eventReadSet, sizeof(fd_set));
    memcpy(&wfds, &gInstance.eventWriteSet, sizeof(fd_set));

    if(flags & NW_EVENT_LOOP_NONBLOCK)
      NW_EVENT_TIMER_CLEAR(pTimeval);
    else
      nwEventGetNextTimeout(&pTimeval); 


    if(pTimeval == NULL)
      NW_DEBG("No timers to serve.");
    else
      NW_DEBG("Next timeout in %u sec %u usec", (unsigned int)pTimeval->tv_sec, (unsigned int)pTimeval->tv_usec);

    rc = select(gInstance.highestFd + 1, &rfds, &wfds, NULL, pTimeval);


    if(rc > 0)
    {
      NW_DEBG("Num of Fds to dispatch %d", rc);
      for(i = 0; i <= gInstance.highestFd ; i++)
      {
        pEvent = NULL;

        if (FD_ISSET(i, &rfds)) {
          pEvent = gInstance.eventReadFdMap[i];
        }

        if (FD_ISSET(i, &wfds)) {
          pEvent = gInstance.eventWriteFdMap[i];
        }

        if(pEvent)
          nwEventInsertIntoActiveQueues(pEvent);
      }
    }
    else 
    {
      if (rc == EBADF)
        NW_CRIT("Select error : EBADF");
      if (rc == EINTR)
        NW_CRIT("Select error : EINTR");
      if (rc == EINVAL)
        NW_CRIT("Select error : EINVAL");
      if (rc == ENOMEM)
        NW_CRIT("Select error : ENOMEM");
    }

    nwEvProcessTimeout(); 

    nwEventDispatch();

  } while(0);

  NW_LEAVE();
  return rc;
}


#ifdef __cplusplus
}
#endif

/*--------------------------------------------------------------------------*
 *                      E N D     O F    F I L E                            *
 *--------------------------------------------------------------------------*/


/*
 * Compile with:
 * cc time-test.c ../src/libNwEvent.a -I ../include/ -o time-test
 */

#include <sys/types.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/stat.h>
#ifndef WIN32
#include <sys/queue.h>
#include <unistd.h>
#endif
#include <time.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "NwEvent.h"

int lasttime;

static void
timeout_cb(void *arg)
{
  struct timeval tv;
  NwEventT *timeout = arg;
  int newtime = time(NULL);

  printf("%s: called at %d: %d\n", __func__, newtime,
      newtime - lasttime);
  lasttime = newtime;

}

int
main (int argc, char **argv)
{
  NwEventRcT rc;
  NwEventT *timeout;

  /* Initalize the event library */
  nwEventInitialize();

  /* Create and start one timer event */
  rc = nwEventTimerCreateAndStart(&timeout, timeout_cb, &timeout, 2, 0, NW_EVENT_TIMER_REPETITIVE);

  nwEventLoop();

  return (0);
}


/*
 * Compile with:
 * cc event-test.c ../src/libNwEvent.a -I ../include/ -o event-test
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "NwEvent.h"

typedef struct 
{
  NwEventT evfifo;
  int socket;
} EventInfoT;

static void
fifo_read(void *arg)
{
  char buf[255];
  int len;
  EventInfoT *eventInfo = arg;

  fprintf(stderr, "fifo_read called with fd: %d, event: %d, arg: %p\n",
      eventInfo->socket, eventInfo->evfifo, arg);

  len = read(eventInfo->socket, buf, sizeof(buf) - 1);

  if (len == -1) {
    perror("read");
    return;
  } else if (len == 0) {
    fprintf(stderr, "Connection closed\n");
    return;
  }

  buf[len] = '\0';
  fprintf(stdout, "Read: %s\n", buf);
  /* No need reschedule this event. All events are persitent by default. */
}

int
main (int argc, char **argv)
{
  EventInfoT eventInfo;
  struct stat st;
  const char *fifo = "event.fifo";

  if (lstat (fifo, &st) == 0) {
    if ((st.st_mode & S_IFMT) == S_IFREG) {
      errno = EEXIST;
      perror("lstat");
      exit (1);
    }
  }

  unlink (fifo);
  if (mkfifo (fifo, 0600) == -1) {
    perror("mkfifo");
    exit (1);
  }

  /* Linux pipes are broken, we need O_RDWR instead of O_RDONLY */
#ifdef __linux
  eventInfo.socket = open (fifo, O_RDWR | O_NONBLOCK, 0);
#else
  eventInfo.socket = open (fifo, O_RDONLY | O_NONBLOCK, 0);
#endif

  if (eventInfo.socket == -1) {
    perror("open");
    exit (1);
  }

  fprintf(stderr, "Write data to %s\n", fifo);
  /* Initalize the event library */
  nwEventInitialize();

  /* Initalize one event */
  nwEventAdd(&eventInfo.evfifo, eventInfo.socket, fifo_read, &eventInfo, NW_EVENT_READ);

  nwEventLoop();

  return (0);
}


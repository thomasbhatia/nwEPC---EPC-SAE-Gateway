AM_CFLAGS = -I$(top_srcdir)/src -I../shared -I../include -Wall @AM_CFLAGS@

# The library name to build (note we are building static libs only)
lib_LIBRARIES = libNwEvent.a

# Where to install the headers on the system
libNwEvent_adir = $(includedir)/NwEvent

# The headers 
libNwEvent_a_HEADERS =    \
			../include/queue.h\
			../include/NwEvent.h

# The sources
libNwEvent_a_SOURCES =    \
			$(libNwEvent_a_HEADERS) \
			NwEvent.c



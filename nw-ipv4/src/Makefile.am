# this is example-file: src/mylib/Makefile.am

# additional include paths necessary to compile the C++ library
AM_CFLAGS = -I$(top_srcdir)/src -I../shared -I../include @AM_CFLAGS@

###############################################################################
# THE LIBRARIES TO BUILD
###############################################################################

# the library names to build (note we are building static libs only)
lib_LIBRARIES = libNwIpv4.a

# where to install the headers on the system
libNwIpv4_adir = $(includedir)/mylib

# the list of header files that belong to the library (to be installed later)
libNwIpv4_a_HEADERS =    \
			../shared/NwTypes.h\
			../shared/NwUtils.h\
			../shared/NwIpv4Error.h\
			../shared/NwIpv4.h\
			../include/NwIpv4Private.h\
			../include/NwIpv4Log.h\
			../shared/NwIpv4Ie.h\
			../shared/NwIpv4Msg.h\
			../include/NwIpv4TunnelEndPoint.h\
			../include/queue.h\
			../include/tree.h

# the sources to add to the library and to add to the source distribution
libNwIpv4_a_SOURCES =    \
			$(libNwIpv4_a_HEADERS) \
			NwIpv4TunnelEndPoint.c\
			NwIpv4Msg.c\
			NwIpv4.c

###############################################################################



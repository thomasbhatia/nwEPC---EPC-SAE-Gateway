# this is example-file: src/mylib/Makefile.am

# additional include paths necessary to compile the C++ library
AM_CFLAGS = -Wall -I$(top_srcdir)/src -I../../common -I../shared -I../include -I../../nw-gtpv1u/shared -I../../nw-gre/shared -I../../nw-ipv4/shared @AM_CFLAGS@

###############################################################################
# THE LIBRARIES TO BUILD
###############################################################################

# the library names to build (note we are building static libs only)
lib_LIBRARIES = libNwSdp.a

# where to install the headers on the system
libNwSdp_adir = $(includedir)/mylib

# the list of header files that belong to the library (to be installed later)
libNwSdp_a_HEADERS =    \
			../../common/NwTypes.h\
			../shared/NwUtils.h\
			../shared/NwSdpError.h\
			../shared/NwLog.h\
			../include/NwSdpLog.h\
			../include/NwSdpPrivate.h\
			../include/NwSdpFlowContext.h\
			../include/queue.h\
			../include/tree.h

# the sources to add to the library and to add to the source distribution
libNwSdp_a_SOURCES =    \
			$(libNwSdp_a_HEADERS) \
			NwSdpFlowContext.c \
			NwSdp.c

###############################################################################



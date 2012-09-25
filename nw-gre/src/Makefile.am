# this is example-file: src/mylib/Makefile.am

# additional include paths necessary to compile the C++ library
AM_CFLAGS = -I$(top_srcdir)/src -I../shared -I../include @AM_CFLAGS@

###############################################################################
# THE LIBRARIES TO BUILD
###############################################################################

# the library names to build (note we are building static libs only)
lib_LIBRARIES = libNwGre.a

# where to install the headers on the system
libNwGre_adir = $(includedir)/mylib

# the list of header files that belong to the library (to be installed later)
libNwGre_a_HEADERS =    \
			../shared/NwTypes.h\
			../shared/NwUtils.h\
			../shared/NwGreError.h\
			../shared/NwGre.h\
			../include/NwGrePrivate.h\
			../include/NwGreLog.h\
			../include/NwGreTrxn.h\
			../shared/NwGreIe.h\
			../shared/NwGreMsg.h\
			../include/NwGreTunnelEndPoint.h\
			../include/queue.h\
			../include/tree.h

# the sources to add to the library and to add to the source distribution
libNwGre_a_SOURCES =    \
			$(libNwGre_a_HEADERS) \
			NwGreTunnelEndPoint.c\
			NwGreTrxn.c\
			NwGreMsg.c\
			NwGre.c

###############################################################################



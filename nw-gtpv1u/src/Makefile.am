# this is example-file: src/mylib/Makefile.am

# additional include paths necessary to compile the C++ library
AM_CFLAGS = -I$(top_srcdir)/src -I../shared -I../include @AM_CFLAGS@

###############################################################################
# THE LIBRARIES TO BUILD
###############################################################################

# the library names to build (note we are building static libs only)
lib_LIBRARIES = libNwGtpv1u.a

# where to install the headers on the system
libNwGtpv1u_adir = $(includedir)/mylib

# the list of header files that belong to the library (to be installed later)
libNwGtpv1u_a_HEADERS =    \
			../shared/NwTypes.h\
			../shared/NwUtils.h\
			../shared/NwGtpv1uError.h\
			../shared/NwLog.h\
			../shared/NwGtpv1u.h\
			../include/NwGtpv1uPrivate.h\
			../include/NwGtpv1uLog.h\
			../include/NwGtpv1uTrxn.h\
			../shared/NwGtpv1uIe.h\
			../shared/NwGtpv1uMsg.h\
			../include/NwGtpv1uTunnelEndPoint.h\
			../include/queue.h\
			../include/tree.h

# the sources to add to the library and to add to the source distribution
libNwGtpv1u_a_SOURCES =    \
			$(libNwGtpv1u_a_HEADERS) \
			NwGtpv1uTunnelEndPoint.c\
			NwGtpv1uTrxn.c\
			NwGtpv1uMsg.c\
			NwGtpv1u.c

###############################################################################



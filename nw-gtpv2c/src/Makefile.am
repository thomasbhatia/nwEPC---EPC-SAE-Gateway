# this is example-file: src/mylib/Makefile.am

# additional include paths necessary to compile the C++ library
AM_CFLAGS = -Wall -I$(top_srcdir)/src -I../shared -I../include @AM_CFLAGS@

###############################################################################
# THE LIBRARIES TO BUILD
###############################################################################

# the library names to build (note we are building static libs only)
lib_LIBRARIES = libNwGtpv2c.a

# where to install the headers on the system
libNwGtpv2c_adir = $(includedir)/mylib

# the list of header files that belong to the library (to be installed later)
libNwGtpv2c_a_HEADERS =    \
			../shared/NwTypes.h\
			../shared/NwUtils.h\
			../shared/NwError.h\
			../shared/NwLog.h\
			../shared/NwGtpv2c.h\
			../shared/NwGtpv2cMsg.h\
			../shared/NwGtpv2cMsgParser.h\
			../shared/NwGtpv2cIe.h\
			../include/NwGtpv2cPrivate.h\
			../include/NwGtpv2cTrxn.h\
			../include/NwGtpv2cTunnel.h\
			../include/NwGtpv2cMsgIeParseInfo.h\
			../include/NwGtpv2cLog.h\
			../include/queue.h\
			../include/tree.h

# the sources to add to the library and to add to the source distribution
libNwGtpv2c_a_SOURCES =    \
			$(libNwGtpv2_a_HEADERS) \
			NwGtpv2cTrxn.c\
			NwGtpv2cTunnel.c\
			NwGtpv2cMsg.c\
			NwGtpv2cMsgIeParseInfo.c\
			NwGtpv2cMsgParser.c\
			NwGtpv2c.c

###############################################################################



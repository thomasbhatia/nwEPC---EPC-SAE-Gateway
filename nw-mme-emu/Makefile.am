# this is example-file: src/bin/Makefile.am

# additional include paths necessary to compile the C++ programs
AM_CFLAGS = -I$(top_srcdir)/nw-gtpv2c/shared -I$(top_srcdir)/nw-gtpv1u/shared -I$(top_srcdir)/nw-sdp/shared -I$(top_srcdir)/common -I$(top_srcdir)/include -I$(top_srcdir)/utils/nw-ipv4-if -I$(top_srcdir)/utils/nw-gtpv2c-if -I$(top_srcdir)/utils/nw-gtpu-if -I$(top_srcdir)/utils/nw-udp -I$(top_srcdir)/utils/nw-log -I$(top_srcdir)/utils/nw-tmr -I$(top_srcdir)/nw-event/include @AM_CFLAGS@

###############################################################################
# THE PROGRAMS TO BUILD
###############################################################################

# the program to build (the names of the final binaries)
bin_PROGRAMS = nwLteMmeEmu

# list of sources for the 'NwWimaxBsEmu' binary
nwLteMmeEmu_SOURCES =    \
                        ../common/tree.h\
                        ../common/queue.h\
                        ../common/NwTypes.h\
                        ../common/NwError.h\
                        ../common/NwLog.h\
                        ../common/NwUtils.h\
                        ../common/NwMem.h\
                        ../utils/nw-ipv4-if/NwIpv4IfLog.h\
                        ../utils/nw-ipv4-if/NwIpv4If.h\
                        ../utils/nw-gtpu-if/NwGtpv1uIfLog.h\
                        ../utils/nw-gtpu-if/NwGtpv1uIf.h\
                        ../utils/nw-tmr/NwTmrMgr.h\
                        ../utils/nw-log/NwLogMgr.h\
                        ../utils/nw-udp/NwUdpLog.h\
                        ../utils/nw-udp/NwUdp.h\
                        ../utils/nw-tmr/NwTmrMgr.c\
                        ../utils/nw-log/NwLogMgr.c\
                        ../utils/nw-udp/NwUdp.c\
                        ../utils/nw-ipv4-if/NwIpv4If.c\
                        ../utils/nw-gtpu-if/NwGtpv1uIf.c\
                        ../nw-gtpv2c/shared/NwGtpv2c.h\
                        ../nw-gtpv2c/shared/NwGtpv2cIe.h\
                        ../nw-gtpv2c/shared/NwGtpv2cMsg.h\
                        ../nw-gtpv2c/shared/NwGtpv2cMsgParser.h\
                        ../common/NwMem.c\
                        NwMmeLog.h\
                        NwMmeUeLog.h\
			NwMmeUlp.h\
                        NwMmeDpe.h\
                        NwMmeUe.h\
			NwMmeUlp.c\
			NwMmeDpe.c\
                        NwMmeMain.c\
                        NwMmeUe.c

# the additional libraries needed to link nwWimaxBsEmu 
nwLteMmeEmu_LDADD =  $(top_builddir)/nw-gtpv2c/src/libNwGtpv2c.a $(top_builddir)/nw-sdp/src/libNwSdp.a $(top_builddir)/nw-gtpv1u/src/libNwGtpv1u.a $(top_builddir)/nw-gre/src/libNwGre.a  $(top_builddir)/nw-ipv4/src/libNwIpv4.a  $(top_builddir)/nw-event/src/libNwEvent.a $(AM_LDFLAGS)

###############################################################################

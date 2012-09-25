bin_PROGRAMS = egtping

AM_CFLAGS = -D__WITH_LIBEVENT__ -I../../shared -I../../include -I../../test-app/nw-udp -I../../test-app/nw-log @AM_CFLAGS@

egtping_SOURCES =    \
			NwEvt.h\
			NwMiniLogMgrEntity.h\
			NwMiniTmrMgrEntity.h\
			NwMiniUlpEntity.h\
			NwMiniUdpEntity.h\
			NwMiniLogMgrEntity.c\
			NwMiniTmrMgrEntity.c\
			NwMiniUlpEntity.c\
			NwMiniUdpEntity.c\
                        NwEgtPingMain.c

egtping_LDADD =  ../../src/libNwGtpv2c.a -levent $(AM_LDFLAGS)

bin_PROGRAMS = helloworld

AM_CFLAGS = -D__WITH_LIBEVENT__ -I../../../nw-gtpv1u/shared -I../../../common -I../../shared -I../..//include -I../../../utils/nw-evt -I../../../nw-event/include @AM_CFLAGS@
#AM_CFLAGS = -I../../../nw-gtpv1u/shared -I../../../common -I../../shared -I../..//include -I$../../test-app/nw-udp -I../../test-app/nw-log @AM_CFLAGS@

helloworld_SOURCES =    \
			NwMiniLogMgrEntity.h\
			NwMiniTmrMgrEntity.h\
			NwMiniUlpEntity.h\
			NwMiniUdpEntity.h\
			NwMiniLogMgrEntity.c\
			NwMiniTmrMgrEntity.c\
			NwMiniUlpEntity.c\
			NwMiniUdpEntity.c\
                        helloworld.c

#helloworld_LDADD =  $(top_builddir)/src/libNwSdp.a -levent $(AM_LDFLAGS)
helloworld_LDADD =  ../../src/libNwSdp.a ../../../nw-gtpv1u/src/libNwGtpv1u.a ../../../nw-gre/src/libNwGre.a  ../../../nw-ipv4/src/libNwIpv4.a -levent $(AM_LDFLAGS)

bin_PROGRAMS = helloworld

AM_CFLAGS = -D_WITH_LIBEVENT__ -I../../shared -I../../include -I../../test-app/nw-udp -I../../test-app/nw-log -I../../../nw-event/include @AM_CFLAGS@

helloworld_SOURCES =    \
			NwEvt.h\
			NwMiniLogMgrEntity.h\
			NwMiniTmrMgrEntity.h\
			NwMiniUlpEntity.h\
			NwMiniLlpEntity.h\
			NwMiniLogMgrEntity.c\
			NwMiniTmrMgrEntity.c\
			NwMiniUlpEntity.c\
			NwMiniLlpEntity.c\
                        helloworld.c

helloworld_LDADD =  ../../src/libNwIpv4.a -levent $(AM_LDFLAGS)

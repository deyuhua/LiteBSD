#	@(#)Makefile	8.1 (Berkeley) 6/6/93

PROG=	talk
DPADD=	${LIBOCURSES} ${LIBTERMCAP} ${LIBCOMPAT}
LDADD=	-locurses -ltermlib -lcompat
SRCS=	ctl.c ctl_transact.c display.c get_addrs.c get_names.c \
	init_disp.c invite.c io.c look_up.c msgs.c talk.c
CFLAGS+=-DOLD_CURSES

.include <bsd.prog.mk>

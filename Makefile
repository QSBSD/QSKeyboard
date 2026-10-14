CC?=		cc
PKGS=		gtk+-3.0 xi xtst x11
CFLAGS+=	-O2 -Wall
QSK_CFLAGS!=	pkg-config --cflags ${PKGS}
QSK_LIBS!=	pkg-config --libs ${PKGS}
PROG=		QSKeyboard
OBJS=		main.o xsend.o altab.o hotkey.o touch.o layout.o actions.o window.o \
		lang/lang.o lang/en.o lang/ru.o

${PROG}: ${OBJS}
	${CC} ${CFLAGS} -o ${PROG} ${OBJS} ${QSK_LIBS}

.c.o:
	${CC} ${CFLAGS} ${QSK_CFLAGS} -I. -c $< -o $@

${OBJS}: qsk.h

clean:
	rm -f ${PROG} ${OBJS}

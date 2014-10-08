VERSION = 0.0.0

# paths
PREFIX = /usr
MANPREFIX = ${PREFIX}/share/man

# includes and libs
GTKINC=$(shell pkg-config --cflags gtk+-2.0 webkit-1.0)
GTKLIB=$(shell pkg-config --libs gtk+-2.0 webkit-1.0 )

INCS = -I. -I/usr/include ${GTKINC}
LIBS = -L/usr/lib -lc ${GTKLIB}
# flags
CPPFLAGS = -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS}
CFLAGS = -std=c99 -O2 -Wall -march=native  ${INCS} ${CPPFLAGS}
LDFLAGS = ${LIBS}

# Solaris
#CFLAGS = -fast ${INCS} -DVERSION=\"${VERSION}\"
#LDFLAGS = ${LIBS}

# compiler and linker
CC = gcc

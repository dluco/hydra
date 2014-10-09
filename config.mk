VERSION = 0.0.0

DISTFILES = LICENSE makefile README TODO data/ src/

# paths
PREFIX = /usr
MANPREFIX = ${PREFIX}/share/man

# includes and libs
GTKINC=$(shell pkg-config --cflags gtk+-2.0 webkit-1.0)
GTKLIB=$(shell pkg-config --libs gtk+-2.0 webkit-1.0)
INCS = ${GTKINC}
LIBS = ${GTKLIB}

# flags
CPPFLAGS += -DVERSION=\"${VERSION}\"
CFLAGS += -g -Wall ${INCS} ${CPPFLAGS}
LDFLAGS += ${LIBS}

# Solaris
#CFLAGS = -fast ${INCS} -DVERSION=\"${VERSION}\"
#LDFLAGS = ${LIBS}

# compiler and linker
CC = gcc

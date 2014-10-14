# sb - simple browser
# See LICENSE file for copyright and license details.

include config.mk

SRC = sb.c callbacks.c
OBJ = ${SRC:.c=.o}

all: sb

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

sb: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@${RM} sb ${OBJ} sb-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p sb-${VERSION}
	@cp -R Makefile config.mk sb sb-binary ${SRC} sb-${VERSION}
	@tar -cf sb-${VERSION}.tar sb-${VERSION}
	@gzip sb-${VERSION}.tar
	@rm -rf sb-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f sb ${DESTDIR}${PREFIX}/bin/sb-binary
	@chmod 755 ${DESTDIR}${PREFIX}/bin/sb-binary
	@cp -f sb-script ${DESTDIR}${PREFIX}/bin/sb
	@chmod 755 ${DESTDIR}${PREFIX}/bin/sb



uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/sb
	@rm -f ${DESTDIR}${PREFIX}/bin/sb-binary


.PHONY: all clean dist install uninstall

# hydra - simple browser
# See LICENSE file for copyright and license details.

include config.mk

SRC = hydra.c callbacks.c
OBJ = ${SRC:.c=.o}

all: hydra

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

hydra: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@${RM} hydra ${OBJ} hydra-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p hydra-${VERSION}
	@cp -R Makefile config.mk hydra hydra-binary ${SRC} hydra-${VERSION}
	@tar -cf hydra-${VERSION}.tar hydra-${VERSION}
	@gzip hydra-${VERSION}.tar
	@rm -rf hydra-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f hydra ${DESTDIR}${PREFIX}/bin/hydra-binary
	@chmod 755 ${DESTDIR}${PREFIX}/bin/hydra-binary
	@cp -f hydra-script ${DESTDIR}${PREFIX}/bin/hydra
	@chmod 755 ${DESTDIR}${PREFIX}/bin/hydra



uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/hydra
	@rm -f ${DESTDIR}${PREFIX}/bin/hydra-binary


.PHONY: all clean dist install uninstall

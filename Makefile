# hydra - lightweight browser
# See LICENSE file for copyright and license details.

include config.mk

all: hydra

hydra:
	@${MAKE} -C src/ $@
#	For convenience, create link to executable
	@ln -sf src/hydra hydra

clean:
	@echo cleaning
	@${RM} hydra hydra-${VERSION}.tar.gz
	@${MAKE} -C src/ $@

strip: all
	@echo striping executable
	@${MAKE} -C src/ $@

dist: clean
	@echo creating dist tarball
	@mkdir hydra-${VERSION}/
	@cp -r ${DISTFILES} hydra-${VERSION}/
	@tar -czf hydra-${VERSION}.tar.gz hydra-${VERSION}/
	@rm -rf hydra-${VERSION}/

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@install -D -m755 src/hydra ${DESTDIR}${PREFIX}/bin/hydra

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/hydra

.PHONY: all hydra clean strip dist install uninstall

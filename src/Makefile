# hydra - lightweight browser
# See LICENSE file for copyright and license details.

include ../config.mk

SRC = hydra.c callbacks.c
OBJ = ${SRC:.c=.o}

all: hydra

${OBJ}: config.mk

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

hydra: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@${RM} hydra ${OBJ}

strip: all
	@strip --strip-all hydra

.PHONY: all clean

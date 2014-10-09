# hydra - lightweight browser
# See LICENSE file for copyright and license details.

include ../config.mk

SRC = args.c browser.c tab.c callbacks.c utils.c
OBJ = ${SRC:.c=.o}

all: hydra

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

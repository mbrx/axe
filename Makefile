CC = gcc
CFLAGS = -std=c99 -ggdb -O0
LDFLAGS =  -ggdb -O0
INSTALL_DIR = /usr/local/bin

OBJECTS = axe.o ent.o commands.o

all:	axe

axe:	$(OBJECTS)
	${CC} ${LDFLAGS} $(OBJECTS) -o axe

%.o:	%.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -f axe *.o

install: axe
	install axe ${INSTALL_DIR}/

uninstall:
	if [ -f ${INSTALL_DIR}/axe ]; then rm ${INSTALL_DIR}/axe; fi

check-syntax:
	${CC} ${CFLAGS} -o nul -S ${CHK_SOURCES}

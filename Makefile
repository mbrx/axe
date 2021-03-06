CC = gcc
CFLAGS = -std=c99 -ggdb -O0
LDFLAGS =  -ggdb -O0
INSTALL_DIR = /usr/local/bin

BINARIES = axe timber
AXE_OBJECTS = axe.o ent.o commands.o
TIMBER_OBJECTS = timber.o

all:	${BINARIES}

axe:	$(AXE_OBJECTS)
	${CC} ${LDFLAGS} $(AXE_OBJECTS) -o axe

timber:	$(TIMBER_OBJECTS)
	${CC} ${LDFLAGS} $(TIMBER_OBJECTS) -o timber


%.o:	%.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm -f axe *.o

install: ${BINARIES}
	install ${BINARIES} ${INSTALL_DIR}/

uninstall:
	if [ -f ${INSTALL_DIR}/axe ]; then rm ${INSTALL_DIR}/axe; fi

check-syntax:
	${CC} ${CFLAGS} -o nul -S ${CHK_SOURCES}

CC = gcc
CFLAGS = -std=c99

axe:	axe.c
	${CC} ${CFLAGS} axe.c -o axe

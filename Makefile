SOURCES = main.c
	
OBJS    = ${SOURCES:.c=.o}
HDRS	= opcodes.h

COPTIONS  =-O2
LOPTIONS   =
CC      = gcc
PACKAGE = gbz80asm

all : ${OBJS} ${HDRS}
	${CC} -o ${PACKAGE} ${OBJS} ${COPTIONS} ${LOPTIONS}

.c.o:
	${CC} ${COPTIONS} ${LOPTIONS} -c $<

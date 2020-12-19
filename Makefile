SOURCES = main.c routine.c jumps.c
	
OBJS    = ${SOURCES:.c=.o}
HDRS	= opcodes.h routine.h jumps.h

COPTIONS  =-O2
LOPTIONS   =
CC      = gcc
PACKAGE = gbz80asm

all : ${OBJS} ${HDRS}
	${CC} -o ${PACKAGE} ${OBJS} ${COPTIONS} ${LOPTIONS}

.c.o:
	${CC} ${COPTIONS} ${LOPTIONS} -c $<

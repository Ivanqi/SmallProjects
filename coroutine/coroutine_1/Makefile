CC=gcc -g -I./
TARGET=coroutine

all:
	${CC} coroutine.c main.c -o ${TARGET}

.PHONY: clean

clean:
	rm -f *.o ${TARGET}
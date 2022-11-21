CC=gcc -g  -I ./
TARGET=asyn_network

all:
	${CC} ae_epoll.c ae.c anet.c main.c -o ${TARGET}

.PHONY: clean

clean:
	rm -f *.o ${TARGET}
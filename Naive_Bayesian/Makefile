CC=g++ -g -std=c++11 -I ./
TARGET=nb

all:
	${CC} naive_bayes.cpp main.cpp -o ${TARGET}

.PHONY: clean

clean:
	rm -f *.o ${TARGET}
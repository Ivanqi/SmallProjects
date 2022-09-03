CC = g++
FLAG = -g -I ../../src/
INCLUDE = 
LIBDIR =
LIB = 
BIN = 
TARGET = websocketserver
SRCS = ../../src/base64.cpp ../../src/sha1.cpp ../../src/network_interface.cpp \ 
	   ../../src/debug_log.cpp ../../src/websocket_handler.cpp \
	   ../../src/websocket_request.cpp main.cpp

$(TARGET):$(SRCS:.cpp=.o)
	$(CC) $(FLAG) $(LIBDIR) $(LIB) -o $@ $^
	-rm -f *.o *.d

%.o:%.cpp
	$(CC) $(FLAG) $(INCLUDE) -c -o $@ $<

clean:
	-rm -f *.o *.d
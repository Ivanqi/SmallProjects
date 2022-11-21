PROJECT := $(shell pwd)
SRCDIR  := $(PROJECT)/../../
TEST := $(wildcard $(PROJECT)/main.cpp)
SRCS := $(wildcard $(SRCDIR)/src/*.cpp)
override SRCS := $(SRCS) $(TEST) 

INCLUDE = -I ../../src/

TARGET = websocketserver


$(TARGET): $(SRCS)
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIBS)

clean :
	find . -name '*.o' | xargs rm -f
	find . -name $(TARGET) | xargs rm -f
PROJECT := $(shell pwd)
SRC := $(wildcard $(PROJECT)/*.cpp)

TARGET := mvcc11
CXX := g++
INCLUDE := -I $(PROJECT)
CFLAGS := -std=c++17 -g -Wall
LIBS := -lpthread


$(TARGET): $(SRC)
	$(CXX) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIBS)

clean :
	find . -name '*.o' | xargs rm -f
	find . -name $(TARGET) | xargs rm -f
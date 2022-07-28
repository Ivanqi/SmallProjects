PROJECT := $(shell pwd)
SRCDIR := $(PROJECT)/../../
SRC := $(wildcard $(SRCDIR)/src/*.c)
INCLUDE := -I $(SRCDIR)/src/ -I $(PROJECT)
OBJ := $(wildcard $(PROJECT)/*.c)
override SRC := $(SRC) $(OBJ)

TARGET := coroutine
CC := gcc
CFLAGS := -g -O -std=gnu99

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^

clean:
	find . -name '*.' | xargs rm -f
	find . -name $(TARGET) | xargs rm -f
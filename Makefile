CC=gcc
CFLAGS=-std=gnu99 -Iinclude
LDFLAGS=-lm

SRC=$(wildcard *.c)
OBJS=$(SRC:.c=.o)

.all: raycast

raycast: $(OBJS)

clean:
	rm -rf $(OBJS) raycast

install:
	mkdir -p bin
	mv raycast bin
	$(MAKE) clean

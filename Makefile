CC=gcc
CFLAGS=-std=gnu99 -Iinclude -O2
LDFLAGS=-lm

SRC=$(wildcard *.c)
OBJS=$(SRC:.c=.o)

.all: raycast

raycast: $(OBJS)
	$(CC) $(OBJS) -o raycast $(LDFLAGS)

clean:
	rm -rf $(OBJS) raycast

install:
	mkdir -p bin
	mv raycast bin
	$(MAKE) clean

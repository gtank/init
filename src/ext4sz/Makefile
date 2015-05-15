CC = gcc
CFLAGS = -Wall -Werror -O3

TARGETS = ext4sz

all: $(TARGETS)

ext4sz: ext4.h
	$(CC) $(CFLAGS) -o ext4sz ext4sz.c

install: $(TARGETS)
	cp $^ ../../bin/

clean:
	rm $(TARGETS)

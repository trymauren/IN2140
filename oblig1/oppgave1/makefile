CFLAGS = -g -Wall -Wextra -std=gnu11
BIN = vowelshift

all: clean vowelshift

vowelshift: vowelshift.c
	gcc $(CFLAGS) vowelshift.c -o vowelshift

run:
	./vowelshift

clean:
	-rm -f vowelshift

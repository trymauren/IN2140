CFLAGS = -g -Wall -Wextra -std=gnu11

all: clean eple run

eple: eple.c the_apple.c the_apple.h
	gcc $(CFLAGS) the_apple.c eple.c -o eple

run:
	./eple

clean:
	-rm -f eple

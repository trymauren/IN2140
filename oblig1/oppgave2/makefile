CFLAGS = -g -Wall -Wextra -std=gnu11
BIN = stringoperations

all: stringoperations clean run

stringoperations: stringoperations.c tests.c
	gcc $(CFLAGS) stringoperations.c tests.c -o tests

run:
	./tests

clean:
	-rm -f $(BIN)

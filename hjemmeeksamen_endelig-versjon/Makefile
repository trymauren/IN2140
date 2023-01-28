CFLAGS = -g -std=gnu11 -Wall -Wextra
BINS = upush_client upush_server
OBJS = send_packet.o

all: $(BINS)

upush_client: upush_client.o datastructure_client.o $(OBJS)
	gcc $(CFLAGS) $< datastructure_client.o $(OBJS) -o $@

upush_server: upush_server.o datastructure_server.o $(OBJS)
	gcc $(CFLAGS) $< datastructure_server.o $(OBJS) -o $@

%.o: %.c
	gcc $(CFLAGS) -c $< #-c betyr lag en objekt fil av dette

clean:
	rm -f $(BINS) *.o

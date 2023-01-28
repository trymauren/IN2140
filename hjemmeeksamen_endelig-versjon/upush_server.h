#include "send_packet.h"

#ifndef SERVER_H
#define SERVER_H


#define TO_SERVER 50
#define CONFSIZE 100
#define IPSIZE 50
#define NICKSIZE 21

// Common:
void check_error(int res, char *msg);


// Server outer:

struct client_reg {
    char* nick;                     // Nick to destination
    in_port_t port;                 // Port to destination
    in_addr_t ip_addr;              // Ip to destination
    struct client_reg* next;        // Next in linkedlist
    int time;                       // Meant for heartbeat
};



// Server inner:


// Shuts down by freeing memory and closing filedescriptors
void quit();

// Checks nick for non- ascii characters and white- space
int check_nick(char nick[], int size);

// Deals with message received. Forwards request to reg_client or lookup_client
void request(char buf_cop[]);

// Registers client
void reg_client(int seq_nr, char *nick);

// Finds requested client_reg´s information
void lookup_client(int seq_nr, char *nick);

// Sends ack
void send_reply(char buf[]);



// Clients:


// Makes head. This happens at startup. The head has the of this
// client and is acting as a pointer to start of linked list
struct client_reg* make_head(char* nick);

// Returns client_reg with nick "xx" or NULL if not found
struct client_reg* check(struct client_reg* head, char* nick);

// Adds client to cache
struct client_reg* add(struct client_reg* head, char* nick, in_port_t port, in_addr_t ip_addr);

// Removes client_reg with nick "xx" from cache
int rem(struct client_reg* head, char* nick);

// Freeing the whole cache when client is shut down by "QUIT" or control c
void trash(struct client_reg* head);

// Prints the cache. Not used when task is delivered
void cache_print(struct client_reg* head);

// Not used....
void heartbeat_check(struct client_reg* head, time_t server_time);


#endif

#include "send_packet.h"

#ifndef CLIENT_H
#define CLIENT_H


#define CHATSIZE 1401
#define MSGSIZE 2000
#define TO_SERVER 50
#define CONFSIZE 100
#define NICKSIZE 21
#define IPSIZE 10

// Common:
void check_error(int res, char *msg);

//Client outer:

struct client_msg {
    struct client_msg* next_msg;
    char* msg;  // Textmessage with size < 1400
    int status; // -1 = not sent, 0 = sent but no ack received, 1 = sent and ack received
    int tries;  // Num of times message is sent
};

struct client_reg {
    char* nick;                     // Nick to destination
    in_port_t port;                 // Port to destination
    in_addr_t ip_addr;              // Ip to destination
    struct client_reg* next;        // Next in linkedlist
    struct client_msg* next_msg;    // First message in que
    int expecting;                  // Ack number expected
    int in_que;                     // Num of messages in que
    int blocked;                    // Blocked?
};



// Client inner:


// Shuts down by freeing memory and closing filedescriptors
void quit();

// Checks nick for non- ascii characters and white- space
int check_nick(char nick[], int size);

// Reads from stdin and replaces newline character with nullbyte. Also empties stdin
void mod_string(char buf[], int size);

// Parses message from internet. Writes to buf[]. Returns 0 if nick from stdin is blocked
int check_msg(char buf[]);

// Parses ack and registers ack with function reg_ack()
void check_ack(char buf[]);

// Checks if client got destination client in cache. If not, it does lookup to server according to task
// Find is called from a context which makes it block
struct client_reg* find(char tonick[]);

// Does direct lookup to server
// Find is called from a context which makes it block
struct client_reg* find2(char tonick[]);

// Sends messages and deal with packet loss, updating expected ack number, etc
void send_msg(struct client_msg* client_msg, struct client_reg* dest_client);

// Parses user input and prompts user with new attempt if format is wrong or nick is blocked
int parse_user_input(char buf[], char tonick_temp[], char txtmsg_temp[]);

// Main execution loop. Runs until Ctrl+c or receives "QUIT" from stdin
void chat();

// Registers client with server. Chat does not start before this is complete
void startup(char nick[]);



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



// Messages:

// Returns a client_reg with message in que
// (Will always return first client_reg with messages in que. A bit unfair, but...)
struct client_msg* get_first(struct client_reg* target);

// Adds message to destination- que
struct client_msg* add_msg(struct client_reg* target, char msg[]);

// Removes first message in destination- que
void rem_msg(struct client_reg* target);

// Registers ack received, and removes message from client_reg expecting ack
// Returns 1 if any client_reg is waiting for seq_nr "xx" and 0 if not
int reg_ack(struct client_reg* head, int seq_nr);

// Checks if client_reg with nick "xx" is blocked
int is_blocked(struct client_reg* head, char nick[]);

// Blocks client_reg with nick "xx"
int block(struct client_reg* head, char nick[]);

// Unblocks client_reg with nick "xx"
int un_block(struct client_reg* head, char nick[]);


#endif

#include "upush_server.h"

void check_error(int res, char *msg){
  if(res == -1){
    perror(msg);
    quit();
    exit(EXIT_FAILURE);
  }
}

struct client_reg* make_head(char *nick) {

    struct client_reg* head = malloc(sizeof(struct client_reg));
    if(head == NULL) {
        fprintf(stderr,"Malloc fail");
        exit(EXIT_FAILURE);
    }

    head->nick = strdup(nick);
    head->port = 0;
    head->ip_addr = 0;
    head->next = NULL;
    head->time = 0;
    return head;

}

struct client_reg* check(struct client_reg* head, char *nick){

    if(head == NULL) {
        return NULL;
    }

    struct client_reg* current = head;

    while(current) {

        if(!strcmp(current->nick, nick)) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

struct client_reg* add(struct client_reg* head, char *nick, in_port_t port, in_addr_t ip_addr) {

    struct client_reg* found = check(head, nick);
    //if existing
    if(found) {
        found->port = port;
        found->ip_addr = ip_addr;
        found->time = time(NULL);
        return found;
    }

    //if not existing before
    struct client_reg* current = head;

    while(current->next) {
        current = current->next;
    }

    struct client_reg* n_client_reg = malloc(sizeof(struct client_reg));
    if(n_client_reg == NULL) {
        fprintf(stderr,"Malloc fail");
        exit(EXIT_FAILURE);
    }

    n_client_reg->nick = strdup(nick);
    n_client_reg->port = port;
    n_client_reg->ip_addr = ip_addr;
    n_client_reg->next = NULL;
    n_client_reg->time = time(NULL);

    current->next = n_client_reg;

    return n_client_reg;
}

int rem(struct client_reg* head, char* nick) {

    struct client_reg* current = head;

    if(!head) {
        return 1;
    }

    if(head->nick == nick) {
        head = current->next;
        free(current->nick);
        free(current);
    }

    while(current->next) {
        if(!strcmp(current->next->nick, nick)) {

            struct client_reg* temp = current->next;
            current->next = current->next->next;
            free(temp->nick);
            free(temp);
            return 0;
        }
    }
    if(current->nick == nick) {
        free(current->nick);
        free(current);
        return 0;
    }
    return 1;
}

void heartbeat_check(struct client_reg* head, time_t server_time) {

    long limit = 30;

    struct client_reg* current = head->next;

    while(current) {

        time_t client_time = current->time;

        if((server_time - client_time) > limit) {
            rem(head, current->nick);
            return;
        }

        current = current->next;
    }
}

void cache_print(struct client_reg* head) {

    struct client_reg* current = head;

    while(current) {
        printf("Nick: %s\n", current->nick);
        printf("Port: %hu\n", current->port);
        printf("Ip: %u\n", current->ip_addr);
        printf("Time: %u\n", current->time);
        // printf("Expecting: %d\n\n\n\n", current->expecting); //denne gir cond jump ...

        current = current->next;
    }
}

void trash(struct client_reg* head) {

    struct client_reg* temp;

    while (head != NULL) {

        temp = head;
        head = head->next;
        free(temp->nick);
        free(temp);
   }
}

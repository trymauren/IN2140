#include "upush_client.h"

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
    head->next_msg = NULL;
    head->in_que = 0;
    head->expecting = 0;
    head->blocked = 0;
    return head;

}

struct client_reg* check(struct client_reg* head, char *nick){

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
    n_client_reg->next_msg = NULL;
    n_client_reg->in_que = 0;
    n_client_reg->expecting = 0;
    n_client_reg->blocked = 0;


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



struct client_msg* get_first(struct client_reg* target) {


    if(target == NULL) {
        return NULL;
    }

    if(target->next_msg == NULL) {
        return NULL;
    }

    return target->next_msg;

}

void cache_print(struct client_reg* head) {

    struct client_reg* current = head;

    while(current) {
        printf("Nick: %s\n", current->nick);
        printf("Port: %hu\n", current->port);
        printf("Ip: %u\n", current->ip_addr);
        printf("In que: %d\n", current->in_que);
        struct client_msg* msg = current->next_msg;

        while(msg) {
            printf("Message: %s\n", msg->msg);
            printf("\n\n\n\n");
            msg = msg->next_msg;
        }

        current = current->next;
    }
}

void trash(struct client_reg* head) {

    struct client_reg* temp;

    while (head != NULL) {

        struct client_msg* temp_msg;
        struct client_msg* msg_head = head->next_msg;
        // printf("Messages:\n");

        while(msg_head != NULL) {
            temp_msg = msg_head;
            msg_head = msg_head->next_msg;
            free(temp_msg->msg);
            free(temp_msg);
        }
        temp = head;
        head = head->next;
        free(temp->nick);
        free(temp);
   }
}

struct client_msg* add_msg(struct client_reg* target, char msg[]) {

    struct client_msg* fresh = malloc(sizeof(struct client_msg));
    if(fresh == NULL) {
        fprintf(stderr,"Malloc fail");
        exit(EXIT_FAILURE);
    }

    fresh->msg = strdup(msg);
    fresh->status = -1;
    fresh->tries = 0;
    fresh->next_msg = NULL;


    if(target->next_msg == NULL) {
        target->next_msg = fresh;
        target->in_que++;
        return fresh;
    }

    struct client_msg* temp = target->next_msg;

    while(temp->next_msg != NULL) {
        temp = temp->next_msg;
    }

    temp->next_msg = fresh;
    target->in_que++;
    return fresh;

}

void rem_msg(struct client_reg* target) {
    if(target->next_msg) {
        //printf("Rem msg %s\n",target->next_msg->msg);
        struct client_msg* temp = target->next_msg;

        target->next_msg = target->next_msg->next_msg;
        free(temp->msg);
        free(temp);

        target->expecting = 0;
        target->in_que--;
    }
    else {
        fprintf(stderr, "------------- Error: Target does not have next_msg -------------\n");
    }
}

int reg_ack(struct client_reg* head, int seq_nr) {

    struct client_reg* temp = head;

    while(temp) {

        if(temp->expecting == seq_nr) {
            rem_msg(temp);
            return 1;
        }

        temp = temp->next;
    }
    return 0;
}

int is_blocked(struct client_reg* head, char nick[]) {

    struct client_reg* current = head;

    while(current) {

        if(!strcmp(current->nick, nick)) {
            return current->blocked;
        }

        current = current->next;

    }
    return 0;
}

int block(struct client_reg* head, char nick[]) {

    struct client_reg* found = check(head, nick);

    if(found == NULL) {
        printf("------------- '%s' is not in cache -------------\n", nick);
        return 0;
    }

    else {
        if(found->blocked) {
            printf("------------- '%s' already blocked -------------\n", nick);
            return 0;
        }
        else {
            found->blocked = 1;
            printf("------------- '%s' is now blocked -------------\n", nick);
            return 1;
        }
    }
    return 0;
}


int un_block(struct client_reg* head, char nick[]) {

    struct client_reg* found = check(head, nick);

    if(found == NULL) {
        printf("------------- '%s' is not in cache -------------\n", nick);
        return 0;
    }

    else {
        if(!found->blocked) {
            printf("------------- '%s' not blocked -------------\n", nick);
            return 0;
        }
        else {
            found->blocked = 0;
            printf("------------- '%s' is now unblocked -------------\n", nick);
            return 1;
        }
    }
    return 0;
}

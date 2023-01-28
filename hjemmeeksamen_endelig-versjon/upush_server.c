#include "upush_server.h"

struct client_reg* head;
struct sockaddr_in* server_addr;
struct sockaddr_in* src_addr;
int fd;


void quit(){

    trash(head);
    free(server_addr);
    free(src_addr);
    close(fd);
    printf("\n\n\n\n------------- UPUSH_SERVER QUIT -------------\n\n\n\n");
	exit(EXIT_SUCCESS);

}


int check_nick(char nick[], int size) {
    if(size > NICKSIZE) {
        return 0;
    }
    for(int i = 0; i < size; i++) {

        char c = nick[i];

        if(!isascii(c) || !isalpha(c) || isdigit(c) || isspace(c)) {
            return 0;
        }
    }
    return 1;
}


int main(int argc, char const *argv[]) {

    (void) signal(SIGINT, quit);

    if(argc != 3) {
        printf("Skriv: %s <port> <loss probability (as percent)> \n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    int server_port = atoi(argv[1]);

    float loss_prb = atoi(argv[2]) / 100;         //Lost probability as decimal (0-1)

    set_loss_probability(loss_prb);

    printf("\n\n\n\n------------- UPUSH_SERVER STARTED -------------\n\n\n\n");

    head = make_head("head");

    int rc;

    struct sockaddr_in* ser_addr = malloc(sizeof(struct sockaddr_in));
    if(ser_addr == NULL) {
        fprintf(stderr,"Malloc fail");
        exit(EXIT_FAILURE);
    }

    server_addr = ser_addr;


    struct sockaddr_in* sc_addr = malloc(sizeof(struct sockaddr_in));
    if(sc_addr == NULL) {
        fprintf(stderr,"Malloc fail");
        exit(EXIT_FAILURE);
    }

    src_addr = sc_addr;

    socklen_t addr_len;
    addr_len = sizeof(struct sockaddr_in);

    char buf[TO_SERVER];

    fd = socket(AF_INET, SOCK_DGRAM, 0); // Lager socket til mottak
    check_error(fd,"socket");

    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(server_port);
    server_addr->sin_addr.s_addr = INADDR_ANY; //Lytt på alt

    rc = bind(fd, (struct sockaddr*)server_addr, sizeof(struct sockaddr_in));
    check_error(rc, "bind");


    while(1){

        // heartbeat_check(head, time(NULL));

        rc = recvfrom(fd, buf, TO_SERVER - 1, 0, (struct sockaddr*)src_addr, &addr_len);
        check_error(rc, "recvfrom");
        buf[rc] = 0;

        char buf_cop[strlen(buf)];
        strcpy(buf_cop,buf);

        request(buf_cop);

        //cache_print(head);

    }
    printf("\n\nQUITTING\n\n");
    quit();
    return EXIT_SUCCESS;
}


void request(char buf_cop[]) {
    int ok = 1;
    char delim[] = " ";

	char *pkt = strtok(buf_cop, delim);

    int seq_nr = atoi(strtok(NULL, delim));

    char *type = strtok(NULL, delim);

    char *nick = strtok(NULL, delim);

    // Checking that server receives message on form "PKT (..)"
    if(strcmp(pkt,"PKT")) {
        ok = 0;
    }

    if(strlen(nick) > 20) {
        ok = 0;
    }

    if(!check_nick(nick, strlen(nick))) {
        fprintf(stderr, "Nick received contains non- ASCII characters\n");
    }

    // checking which request
    if(!strcmp(type,"REG")) {
        reg_client(seq_nr,nick);
    }

    else if(!strcmp(type,"LOOKUP")) {
        lookup_client(seq_nr,nick);
    }

    else {
        ok = 0;
    }

    if(!ok) {
        fprintf(stderr, "Server received packet with wrong format");
    }

}


void reg_client(int seq_nr, char *nick) {

    printf("Request: register\n");
    int wc;
    in_addr_t ip = src_addr->sin_addr.s_addr;
    in_port_t port = src_addr->sin_port;
    struct client_reg* registered = add(head,nick,port,ip);
    if(!registered) {
        fprintf(stderr,"add() feilet");
    }
    char buf[CONFSIZE];
    wc = sprintf(buf,"ACK %d OK",seq_nr);
    check_error(wc,"sprintf");
    send_reply(buf);

}


void lookup_client(int seq_nr, char *nick) {
    printf("Request: lookup\n");
    char buf[CONFSIZE];
    int wc;
    struct client_reg* found = check(head,nick);

    if(found) {
        in_addr_t ip = found->ip_addr;
        in_port_t port = found->port;
        wc = sprintf(buf,"ACK %d NICK %s IP %u PORT %hu", seq_nr, nick, ip, port);
    }
    else {
        wc = sprintf(buf,"ACK %d NOT FOUND", seq_nr);
    }
    check_error(wc,"sprintf");
    send_reply(buf);
}


void send_reply(char buf[]) {
    int wc; // printf("Vi mottok: %s\n",buf);
    wc = send_packet(fd,
                buf,
                strlen(buf),
                0,
                (struct sockaddr*)src_addr,
                sizeof(struct sockaddr_in));
    check_error(wc, "send_packet");
}

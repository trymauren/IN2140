#include "upush_client.h"


int SEQ_NR = 0;
int timeout = 0;
int awaiting_server_ack = 0;

struct client_reg* head;
struct sockaddr_in* this_addr;
struct sockaddr_in* server_addr;
int fd = 0;


void quit() {

    trash(head);
    free(server_addr);
    free(this_addr);
    close(fd);
    printf("\n\n\n\n------------- UPUSH_CLIENT QUIT -------------\n\n\n\n");
	exit(EXIT_SUCCESS);

}


void mod_string(char buf[], int size){

    char c;
    fgets(buf, size, stdin);

    if (buf[strlen(buf) - 1] == '\n') {
        buf[strlen(buf) - 1] = 0;
    }
    else {
        while((c = getchar()) != '\n' && c != EOF);
    }
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


int check_str(char *text) {

    char *ptr;
    for(ptr = text; *ptr != '\0'; ptr ++){
        char c = ptr[0];
        if(!isascii(c)) {
            return 0;
        }
    }
    return 1;
}


int check_msg(char buf[]) {

    int wc;
    int wrong_name = 0;
    int wrong_format = 0;

    char delim[] = " ";

    char text_buf[CHATSIZE];
    strcpy(text_buf,buf);

    char* ptr = strtok(buf, delim);
    if(!ptr) {
        wrong_format = 1;
    }

    int seq_nr_rec = atoi(strtok(NULL, delim));
    if(!seq_nr_rec) {
        wrong_format = 1;
    }

    char *from = strtok(NULL, delim);
    if(!from) {
        wrong_format = 1;
    }

    char *from_nick = strtok(NULL, delim);
    if(!from_nick) {
        wrong_format = 1;
    }
    else {
        if(is_blocked(head, from_nick)) {
            return 0;
        }
    }
    char *to = strtok(NULL, delim);
    if(!to) {
        wrong_format = 1;
    }

    char *to_nick = strtok(NULL, delim);
    if(!to_nick) {
        wrong_format = 1;
    }

    char *msg = strtok(NULL, delim);
    if(!msg) {
        wrong_format = 1;
    }

    char delim_text[] = "0";
    char *text = strtok(NULL, delim_text);
    if(!text) {
        wrong_format = 1;
    }
    else{
        if(!check_str(text)) {
            wrong_format = 1;
        }
    }


    buf[0] = 0;

    if(strcmp(head->nick, to_nick)) {
        wrong_name = 1;
    }

    else if(!check_nick(to_nick, strlen(to_nick))) {
        wrong_format = 1;
    }

    else if( (strcmp(from,"FROM"))
            || (strlen(from_nick) > 20)
            || (strcmp(to,"TO"))
            || (strcmp(msg,"MSG"))
            || (strlen(text) > 1400))
    {
        wrong_format = 1;
    }


    memset(buf,0,sizeof(&buf));

    if(wrong_name) {
        wc = sprintf(buf, "ACK %d WRONG NAME", seq_nr_rec);
        check_error(wc, "sprintf");
        fprintf(stderr, "Packet received has wrong name\n");
        return 1;
    }

    else if(wrong_format) {
        wc = sprintf(buf, "ACK %d WRONG FORMAT", seq_nr_rec);
        check_error(wc, "sprintf");
        fprintf(stderr, "Packet received has wrong format\n");
        return 1;
    }

    else {

        printf("\r%105s [%s]\n", text, from_nick);
        wc = sprintf(buf,"ACK %d OK", seq_nr_rec);
        check_error(wc, "sprintf");
        return 1;
    }
}


void check_ack(char buf[]) {

    int wrong_format = 0;

    char buf_cop[MSGSIZE];
    strcpy(buf_cop, buf);

    char delim[] = " ";
    char* ptr = strtok(buf_cop, delim); //type
    if(!ptr) wrong_format = 1;

    int seq_rec = atoi(strtok(NULL, delim)); //seq
    if(!seq_rec) wrong_format = 1;

    char* ok_wrong = strtok(NULL, delim); //ok/wrong
    if(!ok_wrong) wrong_format = 1;

    if(!strcmp(ok_wrong,"WRONG")) {

        char* name_format = strtok(NULL, delim);

        if(!name_format) {
            fprintf(stderr,"ACK received confirmed message fault\n");
        }
        else {
            if(!strcmp(name_format, "FORMAT")) {
                fprintf(stderr,"ACK received confirmed wrong format\n");

            }
            else if(!strcmp(name_format, "NAME")) {
                fprintf(stderr,"ACK received confirmed wrong name\n");
            }
            else {
                fprintf(stderr,"ACK received confirmed wrong format\n");
            }
        }
    }
    else{
        if(wrong_format) {
            fprintf(stderr,"ACK received confirmed wrong format\n");
        }
    }
    reg_ack(head, seq_rec);

}


struct client_reg* find(char tonick[]) {

    struct client_reg* dest = check(head,tonick);

    if(dest) {
        return dest;
    }
    else{

        fd_set fdset;
        int ready, rc, wc;

        SEQ_NR ++;
        int seq_nr = SEQ_NR;

        char reply[CONFSIZE];

        char not_found_msg[CONFSIZE];
        wc = sprintf(not_found_msg,"ACK %d NOT FOUND",seq_nr);
        check_error(wc,"sprintf");

        char found_str_msg[CONFSIZE];
        wc = sprintf(found_str_msg,"ACK %d NICK %s",seq_nr, tonick);
        check_error(wc,"sprintf");

        char lookup_msg[TO_SERVER];
        wc = sprintf(lookup_msg,"PKT %d LOOKUP %s",seq_nr, tonick);
        check_error(wc,"sprintf");

        int count = 0;

        while(count < 3) {

            struct timeval time;
            time.tv_sec = timeout;
            time.tv_usec = 0;

            FD_ZERO(&fdset);
            FD_SET(fd, &fdset);

            wc = send_packet(fd, lookup_msg, strlen(lookup_msg), 0, (struct sockaddr*)server_addr, sizeof(struct sockaddr_in));
            check_error(wc, "send_packet in find function");

            ready = select(FD_SETSIZE, &fdset, NULL, NULL, &time);
            check_error(ready, "select");

            if(FD_ISSET(fd, &fdset)) {
                rc = recv(fd, reply, CONFSIZE - 1, 0);
                check_error(rc, "recv");
                reply[rc] = 0;

                //if server does not have destination client registered:
                if(!strcmp(not_found_msg,reply)) {
                    fprintf(stderr, "NICK %s NOT REGISTERED\n", tonick);
                    return NULL;
                }

                //if server returns address to destination client:
                //split reply to check:

                char reply_cop[strlen(reply)];
                strcpy(reply_cop,reply);
                char delim[] = " ";

                strtok(reply_cop, delim);

                int ack_nr = atoi(strtok(NULL, delim));

                strtok(NULL, delim);

                char *temp_nick = strtok(NULL, delim);

                strtok(NULL, delim);

                in_addr_t ip_addr = (in_addr_t)strtoul((strtok(NULL, delim)),NULL,0);

                strtok(NULL, delim);

                in_port_t port = (in_port_t)strtoul((strtok(NULL, delim)),NULL,0);

                char comp[CONFSIZE];

                wc = sprintf(comp,"ACK %d NICK %s", ack_nr, temp_nick);
                check_error(wc,"sprintf");

                //if server returns address to destination client, make new client_reg:
                if(!strcmp(found_str_msg, comp)) {
                    SEQ_NR++;
                    return add(head, tonick, port, ip_addr);
                }
            }
            count++;
        }
        //if server does not respond to lookup:
        fprintf(stderr,"\n\n\n------------- No response from server -------------\n\n\n");
        quit();
        return NULL;
    }
}


struct client_reg* find2(char tonick[]) {

    fd_set fdset;

    int ready, rc, wc;

    SEQ_NR ++;
    int seq_nr = SEQ_NR;

    char reply[CONFSIZE];

    char not_found_msg[CONFSIZE];
    wc = sprintf(not_found_msg,"ACK %d NOT FOUND",seq_nr);
    check_error(wc,"sprintf");

    char found_str_msg[CONFSIZE];
    wc = sprintf(found_str_msg,"ACK %d NICK %s",seq_nr, tonick);
    check_error(wc,"sprintf");

    char lookup_msg[TO_SERVER];
    wc = sprintf(lookup_msg,"PKT %d LOOKUP %s",seq_nr, tonick);
    check_error(wc,"sprintf");

    int count = 0;

    while(count < 3) {

        struct timeval time;
        time.tv_sec = timeout;
        time.tv_usec = 0;

        FD_ZERO(&fdset);
        FD_SET(fd, &fdset);

        wc = send_packet(fd, lookup_msg, strlen(lookup_msg), 0, (struct sockaddr*)server_addr, sizeof(struct sockaddr_in));
        check_error(wc, "send_packet in find function");

        ready = select(FD_SETSIZE, &fdset, NULL, NULL, &time);
        check_error(ready, "select");

        if(FD_ISSET(fd, &fdset)) {
            rc = recv(fd, reply, CONFSIZE - 1, 0);
            check_error(rc, "recv");
            reply[rc] = 0;

            //if server does not have destination client registered:
            if(!strcmp(not_found_msg,reply)) {
                fprintf(stderr, "NICK %s NOT REGISTERED\n", tonick);
                return NULL;
            }

            //if server returns address to destination client:
            //split reply to check:

            char reply_cop[strlen(reply)];
            strcpy(reply_cop,reply);
            char delim[] = " ";

            strtok(reply_cop, delim);

            int ack_nr = atoi(strtok(NULL, delim));

            char *temp_nick = strtok(NULL, delim);

            strtok(NULL, delim);

            in_addr_t ip_addr = (in_addr_t)strtoul((strtok(NULL, delim)),NULL,0);

            strtok(NULL, delim);

            in_port_t port = (in_port_t)strtoul((strtok(NULL, delim)),NULL,0);

            char comp[CONFSIZE];

            wc = sprintf(comp,"ACK %d NICK %s", ack_nr, temp_nick);
            check_error(wc,"sprintf");

            //if server returns address to destination client, make new client_reg:
            if(!strcmp(found_str_msg, comp)) {
                return add(head, tonick, port, ip_addr);
            }

        }
        count++;
    }
    //if server does not respond to lookup:
    fprintf(stderr,"\n\n\n------------- No response from server -------------\n\n\n");
    quit();
    return NULL;
}


void send_msg(struct client_msg* client_msg, struct client_reg* dest_client) {


    struct sockaddr_in dest_addr;

    int wc;
    int send_it = 1;


    if(client_msg->status == -1) {

        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = dest_client->port;
        dest_addr.sin_addr.s_addr = dest_client->ip_addr;

        client_msg->status = 0;
        client_msg->tries++;
        SEQ_NR++;
        dest_client->expecting = SEQ_NR;

    }

    else if((client_msg->tries == 1) || (client_msg->tries == 3)){

        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = dest_client->port;
        dest_addr.sin_addr.s_addr = dest_client->ip_addr;

        client_msg->tries++;

    }

    else if(client_msg->tries == 2) {

        struct client_reg* target = find2(dest_client->nick);

        if(target) {

            dest_addr.sin_family = AF_INET;
            dest_addr.sin_port = target->port;
            dest_addr.sin_addr.s_addr = target->ip_addr;

            client_msg->tries++;

        }

        else {

            send_it = 0;

        }

    }

    else if(client_msg->tries >= 4) {

        send_it = 0;

    }


    if(send_it) {

        char send_msg[MSGSIZE];
        char msg[CHATSIZE];

        strcpy(msg,client_msg->msg);

        wc = sprintf(send_msg, "PKT %d FROM %s TO %s MSG %s", dest_client->expecting, head->nick, dest_client->nick, msg);
        check_error(wc, "sprintf");

        wc = send_packet(fd, send_msg, strlen(send_msg), 0, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr_in));
        check_error(wc, "send_packet in send_msg function");

    }

    else{
        printf("-------------Message '%s' timed out-------------\n", client_msg->msg);
        rem_msg(dest_client);

    }

}


int parse_user_input(char buf[], char tonick_temp[], char txtmsg_temp[]) {

    mod_string(buf, CHATSIZE);

    char buf_cop[CHATSIZE];

    strcpy(buf_cop, buf);

    if(!strcmp(buf_cop, "QUIT")) {
        return 0;
    }

    char delim[] = " ";

    char *at = strtok(buf_cop, delim); //@

    if(at == NULL) {
        printf("\n\n------------- Wrong format, type: @[nick] [msg] -------------\n\n");
        return 0;
    }

    if(!strcmp(at, "BLOCK")) {
        printf("Debug: in block blokk");
        char* nick_to_block = strtok(NULL, delim);

        if(nick_to_block == NULL) {
            printf("\n\n------------- Type in nick to block, type: BLOCK [nick] -------------\n\n");
            return 0;
        }
        else {
            if(block(head, nick_to_block)) {
                return -1;
            }
            else {
                return 0;
            }
        }
    }

    if(!strcmp(at, "UNBLOCK")) {

        char* nick_to_block = strtok(NULL, delim);

        if(nick_to_block == NULL) {
            printf("\n\n------------- Type in nick to unblock, type: UNBLOCK [nick] -------------\n\n");
            return 0;
        }
        else {
            if(un_block(head, nick_to_block)) {
                return -1;
            }
            else {
                return 0;
            }
        }
    }

    else {

        if(!(at[0] == '@')) {
            printf("\n\n------------- Missing '@', type: @[nick] [msg] -------------\n\n");
            return 0;
        }

        char *tnick = at + 1;

        if((strlen(tnick)) > (NICKSIZE - 1)) {
            printf("\n\n------------- Nick too long, type: @[nick] [msg] -------------\n\n");
            return 0;
        }

        if(*tnick == 0) {
            printf("\n\n------------- Missing nick, type: @[nick] [msg] -------------\n\n");
            return 0;
        }

        else if(!strcmp(tnick,head->nick)) {
            printf("\n\n------------- Cant send message to yourself..., type: @[nick] [msg] -------------\n\n");
            return 0;
        }

        else if(is_blocked(head, tnick)) {
            printf("\n\n------------- Nick [%s] is blocked, type: UNBLOCK [%s], first -------------\n\n", tnick, tnick);
            return 0;
        }

        strcpy(tonick_temp, tnick);


        char *text = strchr(buf, ' ');
        if(text != NULL) {
            text++;
            if(strlen(text) > CHATSIZE - 1) {
                printf("\n\n------------- Message too long (over 1400 characters), type: @[nick] [msg] -------------\n\n");
                return 0;
            }
        }
        else {
            printf("\n\n------------- Wrong format, type: @[nick] [msg] -------------\n\n");
            return 0;
        }

        strcpy(txtmsg_temp,text);
    }

    return 1;
}


void chat() {

    char payload[MSGSIZE];
    payload[0] = 0;
    fd_set fdset;
    int rc, wc;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    struct sockaddr_in src_addr;

    char stored_tonick[NICKSIZE];
    stored_tonick[0] = 0;
    char stored_txtmsg[CHATSIZE];
    stored_txtmsg[0] = 0;
    char buf[CHATSIZE];

    buf[0] = 0;


    while(strcmp(buf, "QUIT")){

        if(stored_tonick[0] != 0) {

            struct client_reg* target = find(stored_tonick);

            if(target) {
                add_msg(target, stored_txtmsg);;
            }

            stored_tonick[0] = 0;
            stored_txtmsg[0] = 0;

        }

        struct client_reg* temp = head;

        while(temp) {

            struct client_msg* msg = get_first(temp);

            if(msg) send_msg(msg, temp);

            temp = temp->next;

        }


        struct timeval time;
        time.tv_sec = timeout;
        time.tv_usec = 0;

        FD_ZERO(&fdset);

        FD_SET(fd, &fdset);
        FD_SET(STDIN_FILENO, &fdset);

        rc = select(FD_SETSIZE, &fdset, NULL, NULL, &time);
        check_error(rc, "select");


        if (FD_ISSET(fd, &fdset)) {


            rc = recvfrom(fd, payload, CHATSIZE - 1, 0, (struct sockaddr*)&src_addr, &addr_len);
            check_error(rc, "read");
            payload[rc] = 0;
            char buf_cop[CHATSIZE];
            strcpy(buf_cop, payload);

            char delim[] = " ";
        	char *type = strtok(buf_cop, delim);

            int seq = atoi(strtok(NULL, delim));

            if(!strcmp(type, "PKT")) {

                char reply_msg[MSGSIZE];
                strcpy(reply_msg, payload);
                if(check_msg(reply_msg)) {
                    wc = send_packet(fd, reply_msg, strlen(reply_msg), 0, (struct sockaddr*)&src_addr, sizeof(struct sockaddr_in));
                    check_error(wc, "send_packet in main loop");
                }
            }

            else if(!strcmp(type, "ACK")) {

                if(seq == awaiting_server_ack) {
                    awaiting_server_ack = 0;
                }
                else {
                    char print_msg[MSGSIZE];
                    strcpy(print_msg, payload);
                    check_ack(print_msg);
                }
            }
            else {
                fprintf(stderr, "Received a message which is not PKT or ACK... \n");
            }
        }

        if (FD_ISSET(STDIN_FILENO, &fdset)) {

            char tonick_temp[NICKSIZE];
            char txtmsg_temp[CHATSIZE];

            wc = parse_user_input(buf, tonick_temp, txtmsg_temp);

            if(wc <= 0) continue;

            struct client_reg* found = check(head, tonick_temp);

            if(found) {
                if(found->in_que) {
                    add_msg(found,txtmsg_temp);
                }

                else {
                    struct client_msg* msg = add_msg(found,txtmsg_temp);
                    send_msg(msg, found);
                }
            }

            else {
                strcpy(stored_txtmsg, txtmsg_temp);
                strcpy(stored_tonick, tonick_temp);
            }
        }

    }

}


void startup(char nick[]) {


    struct timeval time;
    time.tv_sec = timeout;
    time.tv_usec = 0;
    fd_set fdset;

    int ready, rc, wc;

    int seq_nr = SEQ_NR;

    char confirmation[CONFSIZE];

    char reg_msg[TO_SERVER];

    int ok = 0;

    int count = 0;

    while(count < 3) {

        FD_ZERO(&fdset);

        FD_SET(fd, &fdset);

        wc = sprintf(reg_msg,"PKT %d REG %s",seq_nr, nick);
        check_error(wc,"sprintf");

        wc = send_packet(fd, reg_msg, strlen(reg_msg), 0, (struct sockaddr*)server_addr, sizeof(struct sockaddr_in));
        check_error(wc, "send_packet in reg function");


        ready = select(FD_SETSIZE, &fdset, NULL, NULL, &time);
        check_error(ready, "select");

        if(FD_ISSET(fd, &fdset)) {
            rc = recv(fd, confirmation, CONFSIZE - 1, 0);
            check_error(rc, "recv");
            confirmation[rc] = 0;

            char expected[CONFSIZE];
            wc = sprintf(expected,"ACK %d OK",seq_nr);
            check_error(wc,"sprintf");
            if(!strcmp(expected,confirmation)) {
                printf("------------- Client is registered ! ------------- (%s)\n\n",confirmation);
                printf("------------- Chat below: -------------\n\n\n\n");
                ok = 1;
                break;
            }
        }
        if(ok) break;
        seq_nr++;
        count++;
    }
    if(!ok) {
        fprintf(stderr,"------------- Registration timed out/ failed -------------\n");
        quit();
    }


}


int main(int argc, char const *argv[]){

    if (argc != 6) {
      printf("Skriv: %s <nick> <ip-address> <port> <timeout> <loss probability (as percent)> \n", argv[0]);
      return EXIT_SUCCESS;
    }


    (void) signal(SIGINT, quit);

    //Arguments:
    char nick[NICKSIZE] = {0};
    strcpy(nick, argv[1]);
    if(!check_nick(nick, strlen(nick))) {
        printf("NICK NOT ASCII\n");
        return EXIT_SUCCESS;
    }
    char server_ip[IPSIZE] = {0};
    strcpy(server_ip, argv[2]);
    int server_port = atoi(argv[3]);
    timeout = atoi(argv[4]); //in seconds
    float loss_prb = atof(argv[5]) / 100; //Lost probability as decimal (0-1)

    set_loss_probability(loss_prb);

    printf("\n\n\n\n------------- UPUSH_CLIENT STARTED -------------\n\n\n\n");

    struct client_reg* h;
    h = make_head(nick);
    head = h;

    int rc = 0;

    struct sockaddr_in* t_addr = malloc(sizeof(struct sockaddr_in));
    this_addr = t_addr;
    if(this_addr == NULL) {
        fprintf(stderr,"Malloc fail");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in* s_addr = malloc(sizeof(struct sockaddr_in));
    server_addr = s_addr;
    if(server_addr == NULL) {
        fprintf(stderr,"Malloc fail");
        exit(EXIT_FAILURE);
    }
    struct in_addr server_ip_addr;

    //This client:
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    check_error(fd, "socket");

    this_addr->sin_family = AF_INET;
    this_addr->sin_port = htons(0);
    this_addr->sin_addr.s_addr = INADDR_ANY;


    rc = bind(fd, (struct sockaddr*)this_addr, sizeof(struct sockaddr_in));
    check_error(rc,"bind");


    //Server:
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(server_port);

    rc = inet_pton(AF_INET, server_ip, &server_ip_addr);
    check_error(rc, "inet_pton");
    if (rc == 0) {
      fprintf(stderr, "IP address of server not valid: %s\n", argv[2]);
      return EXIT_FAILURE;
    }
    server_addr->sin_addr = server_ip_addr;


    startup(nick);


    chat();

    printf("\n\nQUITTING\n\n");
    quit();
    return EXIT_SUCCESS;

    }

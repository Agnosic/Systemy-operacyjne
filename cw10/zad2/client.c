#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "helper.h"


int sock;
char* name;

void handle_exit(void);
void send_msg(sock_msg msg);
void send_empty(sock_msg_type);
sock_msg get_sock_msg(void);
void delete_msg(sock_msg msg);
void handle_INT(int signo);
void send_done(int id, char* content);
void init_client(char* n, char* variant, char* address);

int main(int argc, char* argv[]){

    if(argc != 4)
        ferr("Podaj nazwe, siec/unix, adres serwera");

    init_client(argv[1], argv[2], argv[3]);
    while(1){
        sock_msg msg = get_sock_msg();

        switch(msg.type){
            case OK:{
                break;
            }
            case PING: {
                send_empty(PONG);
                break;
            }
            case FULL:{
                ferr("Serwer jest pelny");
            }
            case NAME_TAKEN:{
                ferr("Nazwa jest zajeta");
            }
            case WORK:{
                puts("Rozpoczynam prace");
                char* buffer = malloc(200 + 2* msg.size);
                sprintf(buffer, "echo '%s' | awk '{for(x=1;$x;++x)print $x}' | sort | uniq -c", (char*)msg.content);
                FILE* result = popen(buffer, "r");
                sprintf(buffer, "echo '%s' | wc -w", (char*)msg.content);
                FILE* result2 = popen(buffer, "r");
                int n = fread(buffer, 1, 199 + 2*msg.size, result);
                int n2 = fread(&buffer[n], 1, 199 + 2*msg.size - n, result2);
                buffer[n+n2] = '\0';
                // if(result == 0){
                //     free(buffer);
                //     break;
                // }
                puts("Praca zakonczona");
                send_done(msg.id, buffer);
                free(buffer);
                break;

            }
            default: break;
        }
        delete_msg(msg);
        
    }
    return 0;
}

void init_client(char* n, char* variant, char* address){
    atexit(handle_exit);
    signal(SIGINT, handle_INT);

    name = n;

    if(strcmp(variant, "UNIX") == 0){
        char* un_path = address;

        struct sockaddr_un un_addr;
        un_addr.sun_family = AF_UNIX;
        snprintf(un_addr.sun_path, UNIX_PATH_MAX, "%s", un_path);

        struct sockaddr_un client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        client_addr.sun_family = AF_UNIX;
        snprintf(client_addr.sun_path, UNIX_PATH_MAX, "%s", name);

        if((sock = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) ferrno();

        if(bind(sock, (const struct sockaddr*) &client_addr, sizeof(client_addr)) == -1) ferrno();

        if(connect(sock, (const struct sockaddr*) &un_addr, sizeof(un_addr)) == -1) ferrno();
    }
    else if(strcmp(variant, "WEB") == 0){
        strtok(address, ":");
        char* port = strtok(NULL, ":");
        uint32_t in_addr = inet_addr(address);
        if(in_addr == INADDR_NONE) ferr("bad address");
        uint16_t port_num = atoi(port);

        struct sockaddr_in web_addr;
        memset(&web_addr, 0, sizeof(struct sockaddr_in));

        web_addr.sin_family = AF_INET;
        web_addr.sin_addr.s_addr = in_addr;
        web_addr.sin_port = htons(port_num);

        if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) ferrno();
        if(connect(sock, (const struct sockaddr*) &web_addr, sizeof(web_addr)) == -1) ferrno();
    } else{
        ferr("UNIX lub WEB");
    }
    send_empty(REGISTER);

}



void send_msg(sock_msg msg){
    ssize_t head_size = sizeof(msg.type) + sizeof(msg.size) + sizeof(msg.id) + sizeof(msg.name_size);
    ssize_t size = head_size + msg.size + 1 + 1 + msg.name_size;
    int8_t* buff = malloc(size);

    memcpy(buff, &msg.type, sizeof(msg.type));
    memcpy(buff + sizeof(msg.type), &msg.size, sizeof(msg.size));
    memcpy(buff + sizeof(msg.type) + sizeof(msg.size), &msg.name_size, sizeof(msg.name_size));
    memcpy(buff= + sizeof(msg.type) + sizeof(msg.size) + sizeof(msg.name_size), &msg.id, sizeof(msg.id));

    if(msg.size > 0)
        memcpy(buff + head_size, msg.content, msg.size + 1);
    if(msg.name_size > 0)
        memcpy(buff + head_size + msg.size + 1, msg.name, msg.name_size + 1);


    if(write(sock, buff, size) != size)
        ferrno();

    free(buff);
}

void send_empty(sock_msg_type reply){
    sock_msg msg = {reply, 0, strlen(name), 0, NULL, name};
    send_msg(msg);
}

void send_done(int id, char* content){
    sock_msg msg = {WORK_DONE, strlen(content), strlen(name), id, content, name};
    send_msg(msg);
}



sock_msg get_sock_msg(void){
    sock_msg msg;
    ssize_t head_size = sizeof(msg.type) + sizeof(msg.size) + sizeof(msg.id) + sizeof(msg.name_size);
    ssize_t size = head_size + msg.size + 1 + 1 + msg.name_size;
    int8_t* buff = malloc(size);
    recv(sock, buff, head_size, MSG_PEEK);

    memcpy(&msg.type, buff, sizeof(msg.type));
    memcpy(&msg.size, buff + sizeof(msg.type), sizeof(msg.size));
    memcpy(&msg.name_size, buff + sizeof(msg.type) + sizeof(msg.size), sizeof(msg.name_size));
    memcpy(&msg.id, buff + sizeof(msg.type) + sizeof(msg.size) + sizeof(msg.name_size), sizeof(msg.id));

    int8_t* buffer = malloc(size);
    recv(sock, buffer, size, 0);

    if(msg.size > 0){
        msg.content = malloc(msg.size + 1);
        if(msg.content == NULL) ferrno();
        memcpy(msg.content, buffer + head_size, msg.size + 1);
    } else{
        msg.content = NULL;
    }
    if(msg.name_size > 0){
        msg.name = malloc(msg.name_size + 1);
        memcpy(msg.name, buffer + head_size + msg.size + 1, msg.name_size + 1);
    }
    else{
        msg.name = NULL;
    }
    

    return msg;

}

void handle_INT(int signo){
    exit(0);
}

void delete_msg(sock_msg msg){
    if(msg.content != NULL) free(msg.content);
    if(msg.name != NULL) free(msg.name);
}

void handle_exit(void){
    send_empty(UNREGISTER);
    unlink(name);
    shutdown(sock, SHUT_RDWR);
    close(sock);
}
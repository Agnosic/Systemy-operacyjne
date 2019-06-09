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

int epoll;
int un_sock;
int web_sock;
char* unix_path;

uint64_t id;

pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;
client clients[MAX_CLIENTS];

pthread_t server_thread;
pthread_t pinger_thread;


void init_server(char* port, char* unix_path);
void handle_register(int);
void handle_call(int);
void handle_exit(void);
void* pinger_task(void* params);
void* server_task(void* params);
void delete_client(int);
void send_msg(int, sock_msg msg);
void send_empty(int, sock_msg_type);
void delete_sock(int);
int client_number(char*);
sock_msg get_sock_msg(int sock);
void delete_msg(sock_msg msg);
void handle_INT(int signo);

int main(int argc, char* argv[]){

    if(argc != 3)
        ferr("Numer portu TCP/UDP i sciezka gniazda UNIX");

    init_server(argv[1], argv[2]);
    
    struct epoll_event event;

    while(1){
        epoll_wait(epoll, &event, 1, -1);
        if(event.data.fd < 0)
            handle_register((-1) * event.data.fd);
        else
            handle_call(event.data.fd);
        
    }
    return 0;
}

void init_server(char* port, char* un_path){

    atexit(handle_exit);
    signal(SIGINT, handle_INT);

    uint16_t port_num = (uint16_t) atoi(port);
    if(port_num < 1024) ferr("zly numer portu");

    unix_path = un_path;

    struct sockaddr_in web_address;
    memset(&web_address, 0, sizeof(struct sockaddr_in));
    web_address.sin_family = AF_INET;
    web_address.sin_addr.s_addr = inet_addr("0.0.0.0");
    web_address.sin_port = htons(port_num);

    if((web_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) ferrno();
    if(bind(web_sock, (const struct sockaddr *) &web_address, sizeof(web_address))) ferrno();
    if(listen(web_sock, 64) == -1) ferrno();

    struct sockaddr_un un_adress;
    un_adress.sun_family = AF_UNIX;
    snprintf(un_adress.sun_path, UNIX_PATH_MAX, "%s", unix_path);
    if((un_sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) ferrno();
    if(bind(un_sock, (const struct sockaddr *) &un_adress, sizeof(un_adress))) ferrno();
    if(listen(un_sock, MAX_CLIENTS) == -1) ferrno();

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLPRI;

    if((epoll = epoll_create1(0)) == -1) ferrno();

    event.data.fd = (-1) * web_sock;
    if(epoll_ctl(epoll, EPOLL_CTL_ADD, web_sock, &event) == -1) ferrno();
    event.data.fd = (-1) * un_sock;
    if(epoll_ctl(epoll, EPOLL_CTL_ADD, un_sock, &event)) ferrno();

    if(pthread_create(&pinger_thread, NULL, pinger_task, NULL) != 0) ferrno();
    pthread_detach(pinger_thread);

    if(pthread_create(&server_thread, NULL, server_task, NULL) != 0) ferrno();
    pthread_detach(server_thread);
}


void* pinger_task(void* params){
    while(1){
        pthread_mutex_lock(&client_mutex);
        for(int i = 0; i < MAX_CLIENTS; i++){
            if(clients[i].fd == 0)
                continue;
            if(clients[i].inactive)
                delete_client(i);
            else{
                clients[i].inactive = 1;
                send_empty(clients[i].fd, PING);
            }
            
        }
        pthread_mutex_unlock(&client_mutex);
        sleep(20);
    }
}

void* server_task(void* params){
    char buffer[1024];
    while(1){
        int min_i = MAX_CLIENTS;
        int min = 1000000;

        scanf("%1023s", buffer);
        FILE* file = fopen(buffer, "r");
        if(file == NULL){
            fprintf(stderr, "%s\n", strerror(errno));
            continue;
        }
        fseek(file, 0 ,SEEK_END);
        size_t size = ftell(file);
        fseek(file, 0l, SEEK_SET);

        char* file_buffer = malloc(size+1);
        file_buffer[size] = '\0';
        fread(file_buffer, 1, size, file);
        fclose(file);

        pthread_mutex_lock(&client_mutex);
        for(int i = 0; i < MAX_CLIENTS; i++){
            if(clients[i].fd == 0) continue;
            if(min > clients[i].working){
                min = clients[i].working;
                min_i = i;
            }
        }

        if(min_i < MAX_CLIENTS){
            sock_msg msg = {WORK, strlen(file_buffer) + 1, 0, ++id, file_buffer, NULL};
            printf("ROBOTA %lu WYSLANA DO KLIENTA %s\n", id, clients[min_i].name);
            send_msg(clients[min_i].fd, msg);
            clients[min_i].working++;
        }
        else{
            fprintf(stderr, "JEST 0 KLIENTOW\n");
        }
        pthread_mutex_unlock(&client_mutex);
        free(file_buffer);

    }

}

void handle_register(int sock){
    puts("Nowy klient zarejestrowany");
    int client = accept(sock, NULL, NULL);
    if(client == -1) ferrno();
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLPRI;
    event.data.fd = client;
    if(epoll_ctl(epoll, EPOLL_CTL_ADD, client, &event) == -1) ferrno();
}

void handle_call(int sock){
    sock_msg msg = get_sock_msg(sock);
    pthread_mutex_lock(&client_mutex);

    switch(msg.type){
        case REGISTER:{
            sock_msg_type reply = OK;
            int i;
            i = client_number(msg.name);
            if(i != -1) reply = NAME_TAKEN;

            for(i = 0; i < MAX_CLIENTS && clients[i].fd != 0; i++);

            if(i == MAX_CLIENTS) reply = FULL;
            if(reply != OK){
                send_empty(sock, reply);
                delete_sock(sock);
                break;
            }

            clients[i].fd = sock;
            clients[i].name = malloc(msg.name_size + 1);
            if(clients[i].name == NULL) ferrno();
            strcpy(clients[i].name, msg.name);
            clients[i].working = 0;
            clients[i].inactive = 0;
            send_empty(sock, OK);
            break;
        }
        case UNREGISTER:{
            int i;
            for(i = 0; i < MAX_CLIENTS && strcmp(clients[i].name, msg.name) != 0; i++);
            if(i == MAX_CLIENTS) break;
            delete_client(i);
            break;
        }
        case WORK_DONE:{
            int i = client_number(msg.name);
            if(i != -1){
                clients[i].inactive = 0;
                clients[i].working--;
            }
            printf("ROBOTA %lu PRZEZ KLIENTA %s:\n%s\n", msg.id, (char*)msg.name, (char*)msg.content);
            break;
        }
        case PONG: {
            int i = client_number(msg.name);
            if(i != -1)
                clients[i].inactive = 0;
        }
    }

    pthread_mutex_unlock(&client_mutex);
    delete_msg(msg);
    
}

void delete_client(int i){
    delete_sock(clients[i].fd);
    clients[i].fd = 0;
    printf("KLIENT %s ZOSTAL ODLACZONY\n", clients[i].name);
    clients[i].name = NULL;
    clients[i].working = 0;
    clients[i].inactive = 0;
}


void send_msg(int sock, sock_msg msg){
    write(sock, &msg.type, sizeof(msg.type));
    write(sock, &msg.size, sizeof(msg.size));
    write(sock, &msg.name_size, sizeof(msg.name_size));
    write(sock, &msg.id, sizeof(msg.id));
    if(msg.size > 0)
        write(sock, msg.content, msg.size);
    if(msg.name_size > 0)
        write(sock, msg.name, msg.name_size);
}

void send_empty(int sock, sock_msg_type reply){
    sock_msg msg = {reply, 0, 0, 0, NULL, NULL};
    send_msg(sock, msg);
}

void delete_sock(int sock){
    epoll_ctl(epoll, EPOLL_CTL_DEL, sock, NULL);
    shutdown(sock, SHUT_RDWR);
    close(sock);
}

int client_number(char* name){
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i].fd == 0) continue;
        if(strcmp(clients[i].name, name) == 0) return i;
    }
    return -1;
}

sock_msg get_sock_msg(int sock){
    sock_msg msg;
    read(sock, &msg.type, sizeof(msg.type));
    read(sock, &msg.size, sizeof(msg.size));
    read(sock, &msg.name_size, sizeof(msg.name_size));
    read(sock, &msg.id, sizeof(msg.id));
    if(msg.size > 0){
        msg.content = malloc(msg.size + 1);
        read(sock, msg.content, msg.size);
    } else{
        msg.content = NULL;
    }
    if(msg.name_size > 0){
        msg.name = malloc(msg.name_size + 1);
        read(sock, msg.name, msg.name_size);
    } else{
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
    close(web_sock);
    close(un_sock);
    unlink(unix_path);
    close(epoll);
}
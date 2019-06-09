#ifndef _HELPER_H
#define _HELPER_H

#define MAX_CLIENTS 10
#define UNIX_PATH_MAX 108

#include <sys/socket.h>

typedef enum sock_msg_type{
    REGISTER,
    UNREGISTER,
    PING,
    PONG,
    OK,
    NAME_TAKEN,
    FULL,
    WORK,
    WORK_DONE
}sock_msg_type;

typedef struct client{
    int fd;
    char* name;
    struct sockaddr *addr;
    socklen_t addr_len;
    u_int8_t working;
    u_int8_t inactive;
}client;

typedef struct sock_msg{
    u_int8_t type;
    u_int64_t size;
    u_int64_t name_size;
    u_int64_t id;
    void* content;
    char* name;
}sock_msg;

void ferr(char* text);
void ferrno();
struct timeval get_curr_time();
long int time_diff(struct timeval t1, struct timeval t2);
void print_time(struct timeval t);



#endif
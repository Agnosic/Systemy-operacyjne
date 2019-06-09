#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>
#include <sys/time.h>

#include "helper.h"

void ferrno(){
    ferr(strerror(errno));
}

void ferr(char* text){
    assert(text);
    fprintf(stderr, "%s\n", text);
    exit(1);
}

struct timeval get_curr_time(){
    struct timeval t;
    gettimeofday(&t, NULL);
    return t;
}

long int time_diff(struct timeval t1, struct timeval t2){
    return (t2.tv_sec - t1.tv_sec)*1000000 +  (t2.tv_usec - t1.tv_usec);
}

void print_time(struct timeval t){
    printf("%ld,%ld", t.tv_sec, t.tv_usec);
}
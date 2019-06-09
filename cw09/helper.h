#ifndef _HELPER_H
#define _HELPER_H


void ferr(char* text);
void ferrno();
struct timeval get_curr_time();
long int time_diff(struct timeval t1, struct timeval t2);
void print_time(struct timeval t);



#endif
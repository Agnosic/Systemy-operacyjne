#ifndef _HELPER_H
#define _HELPER_H


void ferr(char* text);
void ferrno();
struct timeval getCurrTime();
long int timeDiff(struct timeval t1, struct timeval t2);
void printTime(struct timeval t);



#endif
#define _XOPEN_SOURCE 500
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

static void sig_int(int signum);
static void sig_tstp(int signum);

int show = 1;

int main(){

    struct sigaction sig_tstp_act;
    sig_tstp_act.sa_handler = sig_tstp;
    sigemptyset(&sig_tstp_act.sa_mask);
    sig_tstp_act.sa_flags = 0;
    char buffer[80];

    if(signal(SIGINT, sig_int) == SIG_ERR){
        fprintf(stderr, "Error with SIG_INT");
    }

    if(sigaction(SIGTSTP, &sig_tstp_act, NULL) == -1){
        fprintf(stderr, "Error with SIG_TSTP");
    }

    while(1){
        if(show){
            time_t rawtime;
            time(&rawtime);
            struct tm *timeinfo = localtime(&rawtime);
            strftime(buffer, 80, "%Y.%m.%d %H:%M:%S", timeinfo);
            printf("%s\n", buffer);
        }
        sleep(1);

    }

}

static void sig_int(int signum){
    puts("\nOdebrano sygnal SIGINT");
    exit(1);
}   

static void sig_tstp(int signum){
    if(show){
        show = 0;
        puts("\nOczekuje na CTRL+Z - kontynuacja albo CTRL+C - zakonczenie programu");
    }
    else{
        show = 1;
    }
}

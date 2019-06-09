#define _XOPEN_SOURCE 500
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

static void sig_int(int signum);
static void sig_tstp(int signum);
void spawn();


int show = 1;
pid_t child_pid;

int main(){

    struct sigaction sig_tstp_act;
    sig_tstp_act.sa_handler = sig_tstp;
    sigemptyset(&sig_tstp_act.sa_mask);
    sig_tstp_act.sa_flags = 0;

    if(signal(SIGINT, sig_int) == SIG_ERR){
        fprintf(stderr, "Error with SIG_INT");
    }

    if(sigaction(SIGTSTP, &sig_tstp_act, NULL) == -1){
        fprintf(stderr, "Error with SIG_TSTP");
    }

    spawn();

    while(1){
        pause();
    }
    return 0;

}

static void sig_int(int signum){
    puts("\nOdebrano sygnal SIGINT");
    if(show){
        kill(child_pid, SIGINT);
    }
    exit(1);
}   

static void sig_tstp(int signum){
    if(show){
        show = 0;
        kill(child_pid, SIGINT);
        puts("\nOczekuje na CTRL+Z - kontynuacja albo CTRL+C - zakonczenie programu");
    }
    else{
        show = 1;
        spawn();
    }
}

void spawn(){
    pid_t child_tmp = vfork();
    if(child_tmp == 0){
        execl("./date", "./date", NULL);
    }
    else{
        child_pid = child_tmp;
    }
}

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



static void catch_kill(int signu, siginfo_t* info, void *context);
static void catch_queue(int signu, siginfo_t* info, void *context);
static void catch_rt(int signu, siginfo_t* info, void *context);
void init_signals_action(void(*fun) (int, siginfo_t*, void*));
void init_signalsRT_action(void(*fun) (int, siginfo_t*, void*));

int sig_sum = 0;

int main(int argc, char* argv[]){
    if(argc != 2){
        fprintf(stderr, "Wrong arguments. MODE\n");
    }

    char* mode = argv[1];

    if(strcmp(mode, "KILL") == 0){
        init_signals_action(catch_kill);
    }else if(strcmp(mode, "SIGQUEUE") == 0){
        init_signals_action(catch_queue);
    }
    else if(strcmp(mode, "SIGRT") == 0){
        init_signalsRT_action(catch_rt);
    }else{
        fprintf(stderr, "Unknown mode");
    }
    printf("PID: %d\n", (int)getpid());

    while(1) pause();

    return 0;
}

static void catch_kill(int signu, siginfo_t* info, void *context){
    if(signu == SIGUSR1){
        kill(info->si_pid, SIGUSR1);
    }
    else if(signu == SIGUSR2){
        kill(info->si_pid, SIGUSR2);
        exit(0);
    }
}

static void catch_queue(int signu, siginfo_t* info, void *context){
    if(signu == SIGUSR1){
        sigqueue(info->si_pid, SIGUSR1, info->si_value);
    }
    else if(signu == SIGUSR2){
        kill(info->si_pid, SIGUSR2);
        exit(0);
    }
}

static void catch_rt(int signu, siginfo_t* info, void *context){
    if(signu == SIGRTMIN){
        kill(info->si_pid, SIGRTMIN);
    }
    else if(signu == SIGRTMAX){
        kill(info->si_pid, SIGRTMAX);
        exit(0);
    }
}


void init_signals_action(void(*fun) (int, siginfo_t*, void*)){
    sigset_t signals;

    sigfillset(&signals);

    sigdelset(&signals, SIGUSR1);
    sigdelset(&signals, SIGUSR2);

    sigprocmask(SIG_BLOCK, &signals, NULL);


    struct sigaction act;
    act.sa_sigaction = fun;
    act.sa_mask = signals;
    act.sa_flags = SA_SIGINFO;

    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);
}

void init_signalsRT_action(void(*fun) (int, siginfo_t*, void*)){
    sigset_t signals;

    sigfillset(&signals);

    sigdelset(&signals, SIGRTMAX);
    sigdelset(&signals, SIGRTMIN);

    sigprocmask(SIG_BLOCK, &signals, NULL);


    struct sigaction act;
    act.sa_sigaction = fun;
    act.sa_mask = signals;
    act.sa_flags = SA_SIGINFO;

    sigaction(SIGRTMAX, &act, NULL);
    sigaction(SIGRTMIN, &act, NULL);
}
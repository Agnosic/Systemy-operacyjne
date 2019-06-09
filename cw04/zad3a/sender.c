#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int sig_sum = 0;
int expected_sig_sum = 0;

static void catch_kill(int signu, siginfo_t* info, void *context);
static void catch_queue(int signu, siginfo_t* info, void *context);
static void catch_rt(int signu, siginfo_t* info, void *context);
void init_signals_action(void(*fun) (int, siginfo_t*, void*));
void init_signalsRT_action(void(*fun) (int, siginfo_t*, void*));

int main(int argc, char* argv[]){
    if(argc != 4){
        fprintf(stderr, "Wrong arguments. PID SIGNAL_NUMBER and MODE");
    }
    int pid = atoi(argv[1]);
    expected_sig_sum = atoi(argv[2]);
    char* mode = argv[3];

    if(strcmp(mode, "KILL") == 0){
        init_signals_action(catch_kill);
        for(int i = 0; i < expected_sig_sum; i++){
            kill(pid, SIGUSR1);
        }
        kill(pid, SIGUSR2);
    }else if(strcmp(mode, "SIGQUEUE") == 0){
        init_signals_action(catch_queue);
        for(int i = 0; i < expected_sig_sum; i++){z
            kill(pid, SIGUSR1);
        }
        kill(pid, SIGUSR2);
    }
    else if(strcmp(mode, "SIGRT") == 0){
        init_signalsRT_action(catch_rt);
        for(int i = 0; i < expected_sig_sum; i++){
            kill(pid, SIGRTMIN);
        }
        kill(pid, SIGRTMAX);
    }else{
        fprintf(stderr, "Unknown mode");
    }

    while(1) pause();

    return 0;
}

static void catch_kill(int signu, siginfo_t* info, void *context){
    if(signu == SIGUSR1){
        sig_sum++;
    }
    else if(signu == SIGUSR2){
        printf("expected: %d, real: %d\n", expected_sig_sum, sig_sum);
        exit(0);
    }
}

static void catch_queue(int signu, siginfo_t* info, void *context){
    if(signu == SIGUSR1){
        sig_sum++;
        printf("%d SIGUSR1 signal caughts\n", info->si_value.sival_int);
    }
    else if(signu == SIGUSR2){
        printf("expected: %d, real: %d\n", expected_sig_sum, sig_sum);
        exit(0);
    }
}

static void catch_rt(int signu, siginfo_t* info, void *context){
    if(signu == SIGRTMIN){
        sig_sum++;
    }
    else if(signu == SIGRTMAX){
        printf("expected: %d, real: %d\n", expected_sig_sum, sig_sum);
        exit(0);
    }
}


void init_signals_action(void(*fun) (int, siginfo_t*, void*)){
    sigset_t signals;

    sigfillset(&signals);

    sigdelset(&signals, SIGUSR1);
    sigdelset(&signals, SIGUSR2);



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



    struct sigaction act;
    act.sa_sigaction = fun;
    act.sa_mask = signals;
    act.sa_flags = SA_SIGINFO;

    sigaction(SIGRTMAX, &act, NULL);
    sigaction(SIGRTMIN, &act, NULL);
}
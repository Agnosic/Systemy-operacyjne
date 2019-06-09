#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>


struct child{
    pid_t pid;
    char* file_name;
    int interval;
    int stopped;
}child;

struct children{
    struct child* child_list;
    int count;
}children;


void help();
char* readFile(char* path);
void watch1(char* file, int interval, int list_numb);
int sum_lines(char *string);
pid_t monitor(char* file, int interval, int list_numb);
void parent_sa_handler();
static void sig_parent_handler(int signo);
static void sig_child_handler(int signo);
void list_show();
void stop_child(struct child* child);
void stop_all();
void stop_pid( pid_t child_pid);
void start_child(struct child* child);
void start_all();
void start_pid(pid_t child_pid);
struct child* find_child(pid_t child_pid);

int run = 1;
struct children children;

int main(int argc, char* argv[]){
    parent_sa_handler();
    if(argc !=  2){
        help();
        exit(1);
    }

    char* list = argv[1];
    char* file = malloc(200);


    char* buffer = readFile(list);
    int child_sum = sum_lines(buffer);

    struct child* child_list = calloc(child_sum, sizeof(child));
    int interval;
    char* line = buffer;
    int child_numb = 0;
    while(line){
        char* nextline = strchr(line, '\n');
        if(nextline){
            nextline[0] = '\0';
        }
        if(sscanf(line, "%200s %d", file, &interval)){
            pid_t child_pid = monitor(file, interval, child_numb);
            if(child_pid){
                struct child* child= &child_list[child_numb];
                child->pid = child_pid;
                child->file_name = malloc(200);
                strcpy(child->file_name, file);
                child->interval = interval;
                child_numb++;
                child->stopped = 0;
            }
        }
        if(nextline){
            nextline[0] = '\n';
            if(strcmp(nextline, "\n") == 0){
                line = NULL;
            }
            else{
                line = &nextline[1];
            }
        }

    }


    children.child_list = child_list;
    children.count = child_sum;

    list_show(&children);
    char command[20];

    while(run){
        fgets(command, 20, stdin);
        if(strcmp(command, "LIST\n") == 0){
            list_show(&children);
        }
        else if(strcmp(command, "STOP ALL\n") == 0){
            stop_all();
        }
        else if(strncmp(command, "STOP ", 5) == 0){
            int pid = atoi(command+5);
            stop_pid(pid);
        }
        else if(strcmp(command, "START ALL\n") == 0){
            start_all();
        }
        else if(strncmp(command, "START ", 6) == 0){
            int pid = atoi(command+5);
            start_pid(pid);
        }
        else if(strcmp(command, "END\n") == 0){
            raise(SIGINT);
        }


    }

    for(int i = 0; i < child_sum; i++){
        int exitstatus;
        kill(child_list[i].pid, SIGINT);
        pid_t child_pid = waitpid(child_list[i].pid, &exitstatus, 0);
        if(WIFEXITED(exitstatus)){
            printf("Proces %d utworzyl %d kopii pliku\n", (int) child_pid, WEXITSTATUS(exitstatus));
        }
    }

    free(file);

    free(buffer);
    free(child_list);
    return 0;
}

void help(){
    fprintf(stderr, "\n .\\monitor <list>\n");
}

char* readFile(char* file){
    struct stat stat;
    lstat(file, &stat);

    FILE* fileopen = fopen(file, "r");

    char* buffer = calloc(stat.st_size +1, sizeof(char));
    fread(buffer, 1, stat.st_size, fileopen);
    fclose(fileopen);

    buffer[stat.st_size] = '\0';
    return buffer;
}

int sum_lines(char* string){
    int sum = 0;
    char* c = string;
    while(c){
        c = strchr(c, '\n');
        if(c){
            c++;
            sum++;
        }
    }
    return sum;
}

pid_t monitor(char* file, int interval, int list_numb){
    pid_t child_pid = fork();

    if(child_pid == -1){
        fprintf(stderr, "child pid is -1");
        return 0;
    }
    else if(child_pid == 0){
        struct sigaction act;
        act.sa_handler = sig_child_handler;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(SIGUSR1, &act, NULL);
        sigaction(SIGUSR2, &act, NULL);
        watch1(file, interval, list_numb);
    }


    return child_pid;
}

void watch1(char* file, int interval, int list_numb){
    struct stat stat;
    if(lstat(file, &stat) == -1){
        fprintf(stderr, "\n File does not exists %s\n", file);
        exit(0);
    }
    int n = 0;

    time_t lastmod = stat.st_mtime;

    char* buffer = readFile(file);
    char* fileNew = malloc(strlen(file) + 35);
    //int pid= getpid();
    //struct child* child = find_child(pid);
    strcpy(fileNew, file);
    while(run){
        sleep(interval);
    

        if(lstat(file, &stat) == -1){
            fprintf(stderr, "\n File stopped existing %s\n", file);
            exit(0);
        }

        if(stat.st_mtime > lastmod){
            strftime(&fileNew[strlen(file)], 35, "_%Y-%m-%d_%H-%M-%S", localtime(&lastmod));
            mkdir("archiwum", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            chdir("archiwum");

            FILE* backup = fopen(fileNew, "w");
            fwrite(buffer, sizeof(char), strlen(buffer), backup);
            fclose(backup);
            free(buffer);
            chdir("..");
            buffer = readFile(file);
            lastmod = stat.st_mtime;
            n++;
            fclose(backup);
        }
        //if(child->stopped){
        //   lastmod = stat.st_mtime;
        //}
    }
    free(buffer);
    free(fileNew);
    exit(n);
    
    
}

void parent_sa_handler(){
    struct sigaction act;
    act.sa_handler = sig_parent_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);
}
static void sig_child_handler(int signo){
    if(signo == SIGUSR1){
        puts("child stopped");
        pause();
        
    }
    else{
        puts("child started");
    }
}

static void sig_parent_handler(int signo){
    if(signo == SIGINT){
        run = 0;
    }
}

void list_show(struct children* children){
    for(int i = 0; i< children->count; i++){
        if(children->child_list[i].stopped == 1){
            printf("STOPPED ");
        }
        printf("Process %d is monitoring file %s with interval %d seconds\n",
            children->child_list[i].pid,
            children->child_list[i].file_name,
            children->child_list[i].interval);
    }
}

void stop_child(struct child* child){
    if(child->stopped ){
        fprintf(stderr, "child %d is already stopped\n", child->pid);
        return;
    }
    kill(child->pid, SIGUSR1);

    child->stopped = 1;


}

void stop_all(){
    for(int i = 0; i < children.count; i++){
        stop_child(&children.child_list[i]);
    }
}

struct child* find_child(pid_t child_pid){
    struct child* child;
    for(int i = 0; i< children.count; i++){
        if(children.child_list[i].pid == child_pid){
            child = &children.child_list[i];
        }
    }
    return child;
}

void stop_pid(pid_t child_pid){
    struct child* child = find_child(child_pid);

    stop_child(child);

}

void start_child(struct child* child){
    if(!child->stopped){
        fprintf(stderr, "child %d is already on\n", child->pid);
        return;
    }
    child->stopped = 0;
    kill(child->pid, SIGUSR2);

}

void start_all(){
    for(int i = 0; i < children.count; i++){
        start_child(&children.child_list[i]);
    }
}

void start_pid(pid_t child_pid){
    struct child* child = find_child(child_pid);

    start_child(child);

}


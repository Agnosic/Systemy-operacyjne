#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

void help();
char* readFile(char* path);
void watch1(char* file, int interval, int timesec);
void watch2(char* file, int interval, int timesec);

int monitor(char* file, int interval, int timesec, int mode);

int main(int argc, char* argv[]){
    if(argc !=  4){
        help();
        exit(1);
    }

    char* list = argv[1];
    int timesec = atoi(argv[2]);
    int mode = atoi(argv[3]);
    int child_amount = 0;
    char* file = malloc(200);


    char* buffer = readFile(list);
    int interval;
    char* line = buffer;
    while(line){
        char* nextline = strchr(line, '\n');
        if(nextline){
            nextline[0] = '\0';
        }
        if(sscanf(line, "%200s %d", file, &interval)){
            child_amount += monitor(file, interval, timesec, mode);
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
    for(int i = 0; i < child_amount; i++){
        int exitstatus;
        pid_t child_pid = wait(&exitstatus);
        if(WIFEXITED(exitstatus)){
            printf("Proces %d utworzyl %d kopii pliku\n", (int) child_pid, WEXITSTATUS(exitstatus));
        }
    }

    free(file);

    free(buffer);

    exit(0);
    return 0;
}

void help(){
    fprintf(stderr, "\n .\\monitor <list> <time> <mode>\n");
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

int monitor(char* file, int interval, int timesec, int mode){
    pid_t child_pid = fork();

    if(child_pid == -1){
        fprintf(stderr, "child pid is -1");
        return 0;
    }
    else if(child_pid == 0){
        if(mode == 1){
            watch1(file, interval, timesec);
        }
        else if(mode == 2){
            watch2(file, interval, timesec);
        }
        else{
            fprintf(stderr, "\n Wrong mode. Must be 1 or 2");
        }
    }


    return 1;
}

void watch1(char* file, int interval, int timesec){
    struct stat stat;
    int passedTime = 0;
    if(lstat(file, &stat) == -1){
        fprintf(stderr, "\n File does not exists %s\n", file);
        exit(0);
    }
    int n = 0;

    time_t lastmod = stat.st_mtime;

    char* buffer = readFile(file);
    char* fileNew = malloc(strlen(file) + 35);
    strcpy(fileNew, file);
    //sprintf(fileNew, "archiwum/%s", fileNew);
    while((passedTime += interval) <= timesec){
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
        sleep(interval);
    }
    free(buffer);
    free(fileNew);
    exit(n);
    
    
}

void watch2(char* file, int interval, int timesec){
    struct stat stat;
    int passedTime = 0;
    if(lstat(file, &stat) == -1){
        fprintf(stderr, "\n File does not exists %s\n", file);
        exit(0);
    }
    int n = 0;

    time_t lastmod = stat.st_mtime;


    char* fileNew = malloc(strlen(file) + 25);
    strcpy(fileNew, file);
    mkdir("archiwum", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    chdir("archiwum");
    chdir("..");
    //sprintf(fileNew, "archiwum/%s", fileNew);
    do{
        if(lstat(file, &stat) == -1){
            fprintf(stderr, "\n File stopped existing %s\n", file);
            exit(0);
        }

        if(stat.st_mtime > lastmod || n == 0){
            strftime(&fileNew[strlen(file)], 25, "_%Y-%m-%d_%H-%M-%S", localtime(&lastmod));
            lastmod = stat.st_mtime;
            pid_t child_pid = vfork();
            if(child_pid == -1){
                fprintf(stderr, "child pid was -1. %s didn't get a backup\n", file);
            }
            else if(child_pid == 0){
                char* fileArch = malloc(strlen(fileNew) + 15);
                sprintf(fileArch, "archiwum/%s", fileNew);
                execl("/bin/cp", "cp", file, fileArch, NULL);
                free(fileArch);
                n++;
            }
            else{
                int stat;
                wait(&stat);
                if(stat){
                    fprintf(stderr, "%s didn't get a backup\n", file);
                }
                else{
                    n++;
                }
            }
        }
        sleep(interval);
    }while((passedTime += interval) <= timesec);

    free(fileNew);
    exit(n);
}

#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <ftw.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>

void help();
void tree(char* path, char* path2);
void file_print(const char* path, const char* path2);


char* nftwsymbol;
time_t nftwtim;

int main(int argc, char* argv[]){
    if(argc < 2){
        help();
        exit(0);
    }

    char* path = realpath(argv[1], NULL);
    if(path){
        perror(path);
    }

    tree(path, ".");
    free(path);
    return 0;
}

void help(){
    printf("You can use this commands:\n");  
    printf("<dir> \n\n");
}


void tree(char* path, char* path2){
    file_print(path, path2);
    DIR* dir = opendir(path);
    if(!dir){
        fprintf(stderr, "%s\n", strerror(errno));
        return;
    }
    struct stat stat;
    struct dirent* dirent;
    while((dirent = readdir(dir)) != NULL){
        if(strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0){
            continue;
        }
        char* file_path = malloc(strlen(path) + 2 + strlen(dirent->d_name));
        char* file_path2 = malloc(strlen(path2) + 3 + strlen(dirent->d_name));
        if(!file_path){
            fprintf(stderr, "Could not allocate any more memory");
        }
        if(!file_path2){
            fprintf(stderr, "Could not allocate any more memory");
        }
        sprintf(file_path, "%s/%s", path, dirent->d_name);
        if(strcmp(file_path2, ".")){
            sprintf(file_path2, "./%s", dirent->d_name);
        }
        else{
            sprintf(file_path2, "/%s/%s", path2, dirent->d_name);
        }
        lstat(file_path, &stat);
        
        if(S_ISDIR(stat.st_mode)){
            long int loc = telldir(dir);
            closedir(dir);
            tree(file_path, file_path2);
            opendir(path);
            seekdir(dir, loc);
        }

    }

    if(closedir(dir) == -1){
        fprintf(stderr, "%s\n", strerror(errno));
    }

}

void file_print(const char* path, const char* path2){
    pid_t child_pid;
    child_pid = vfork();


    if(child_pid == -1){
        fprintf(stderr, "child pid is -1");
    }
    else if(child_pid == 0){
        printf("\n\nDIR %s\nPID %d\n", path2, (int)getpid());
        execl("/bin/ls", "ls", "-l", path, NULL);
    }
    else{
        int stat;
        wait(&stat);
        if(stat){
            fprintf(stderr, "Child procces failiure\n");
        }
    }

}




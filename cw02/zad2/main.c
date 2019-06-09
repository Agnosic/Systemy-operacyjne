#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <ftw.h>
#include <errno.h>

void help();
time_t parse_time(char* ctime);
void tree(char* path, char* symbol, time_t tim);
int is_time_valid(char* symbol, time_t tim, time_t ftim);
void file_print(const char* path, const struct stat* stat);
static int nftw_tree(const char* path, const struct stat* sb, int typeflag, struct FTW* ftwbuf);
void tree(char* path, char* symbol, time_t tim);

char* nftwsymbol;
time_t nftwtim;

int main(int argc, char* argv[]){
    if(argc < 5){
        help();
        exit(0);
    }

    char* path = realpath(argv[1], NULL);
    if(path){
        perror(path);
    }

    char* symbol = argv[2];
    time_t tim = parse_time(argv[3]);

    char* mode = argv[4];

    if(strcmp(mode, "1") == 0){
        tree(path, symbol, tim);
    }
    else if(strcmp(mode, "2") == 0){
        nftwsymbol = symbol;
        nftwtim = tim;
        if(nftw(path, nftw_tree, 30, FTW_PHYS) != 0){
            perror("nftw");
        }
    }
    else{
        fprintf(stderr, "Mode must me 1 or 2\n");
        exit(1);
    }
    return 0;
}

void help(){
    printf("You can use this commands:\n");  
    printf("<dir> <'<', '>', '='> <data> <1=normal, 2=nftw>\n\n");
}

static int nftw_tree(const char* path, const struct stat* stat, int typeflag, struct FTW* ftwbuf){
    if(ftwbuf->level == 0){
        return 0;
    }
    if(is_time_valid(nftwsymbol, nftwtim, stat->st_mtime)){
        file_print(path, stat);
    }

    
    return 0;
}

void tree(char* path, char* symbol, time_t tim){
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
        if(!file_path){
            fprintf(stderr, "Could not allocate any more memory");
        }
        sprintf(file_path, "%s/%s", path, dirent->d_name);
        lstat(file_path, &stat);
        if(is_time_valid(symbol, tim, stat.st_mtime)){
            file_print(file_path, &stat);
        }
        if(S_ISDIR(stat.st_mode)){
            long int loc = telldir(dir);
            closedir(dir);
            tree(file_path, symbol, tim);
            opendir(path);
            seekdir(dir, loc);
        }

    }

    if(closedir(dir) == -1){
        fprintf(stderr, "%s\n", strerror(errno));
    }

}

void file_print(const char* path, const struct stat* stat){
    printf("%s\n", path);

    switch(stat->st_mode & S_IFMT){
        case S_IFREG: printf("regular file\n"); break;
        case S_IFDIR: printf("directory\n"); break;
        case S_IFCHR: printf("character device\n"); break;
        case S_IFBLK: printf("block device\n"); break;
        case S_IFIFO: printf("FIFO/pipe\n"); break;
        case S_IFLNK: printf("symlink\n"); break;
        case S_IFSOCK: printf("socket\n"); break;
        default: printf("unknown\n"); break;
    }

    printf("File size:              %lld bytes\n", (long long) stat->st_size);
    printf("Last file access:       %s", ctime(&stat->st_atime));
    printf("Last file modification: %s\n", ctime(&stat->st_mtime));

}

time_t parse_time(char* ctim){
    struct tm tm;
    char* date = strptime(ctim, "%Y-%m-%d %H:%M:%S" , &tm);
    if(date == NULL){
        fprintf(stderr, "bad time format. example 2018-12-12 11:15:56\n");
        exit(1);
    }
    time_t time = mktime(&tm);
    if(time == -1){
        fprintf(stderr, "WTF\n");
        exit(1);
    }
    return time;
}

int is_time_valid(char* symbol, time_t tim, time_t ftim){
    if(strcmp(symbol, ">") == 0){
        return ftim > tim;
    }
    else if(strcmp(symbol, "<") == 0){
        return ftim < tim;
    }
    else if(strcmp(symbol, "=") == 0){
        return ftim == tim;
    }
    else{
        fprintf(stderr, "bad date symbol\n");
        exit(1);
    }
    return 0;
}


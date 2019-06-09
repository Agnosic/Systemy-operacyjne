#include <dlfcn.h>

#ifndef DLL
#include "blocks.h"
#endif



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/times.h>
#include <time.h>

#ifdef DLL
void* dll;
typedef struct{
    char** table;
    int size;
    char* dir;
    char* file;
    char* tmp_file;
}table_block;
#endif

typedef struct{
    double time_real;
    double time_system;
    double time_user;
    clock_t clock_start;
    struct tms start;
}timer;

void help();
void timer_print(timer* t, char* word);
void timer_start(timer* t);
void timer_stop(timer* t);
void timer_reset(timer* t);
double time_diff(clock_t start, clock_t end);

int main(int argc, char* argv[]){
    timer timers[4];
    timer* find_timer = &timers[0];
    timer* alloc_timer = &timers[1];
    timer* remove_timer = &timers[2];
    timer* alternately_timer = &timers[3];
    timer_reset(find_timer);
    timer_reset(alloc_timer);
    timer_reset(remove_timer);
    timer_reset(alternately_timer);
    #ifdef DLL
    dll = dlopen("libblocks.so", RTLD_LAZY);
    if(!dll){
        fprintf(stderr, "Failed at opening dynamic library");
    }
    table_block* (*create_table)(int);
    create_table = dlsym(dll, "create_table");
    void (*remove_table_block)(table_block*);
    remove_table_block = dlsym(dll, "remove_table_block");
    #endif

    if(argc == 1){
        help();
        exit(0);
    }
    int size = atoi(argv[1]);

    table_block* tb = create_table(size);

    int current_arg = 2;
    while(current_arg < argc){
        if(strcmp(argv[current_arg], "-s") == 0 || strcmp(argv[current_arg], "--search_directory") == 0){
            if(current_arg + 3 >= argc){
                break;
            }
            tb->dir = argv[++current_arg];
            tb->file = argv[++current_arg];
            tb->tmp_file = argv[++current_arg];

            #ifdef DLL
            void (*find)(table_block*);
            find = dlsym(dll, "find");
            int (*alloc_block)(table_block*);
            alloc_block = dlsym(dll, "alloc_block");
            #endif

            timer_start(find_timer);
            find(tb);
            timer_stop(find_timer);

            timer_start(alloc_timer);
            alloc_block(tb);
            timer_stop(alloc_timer);

            current_arg++;
        }
        else if(strcmp(argv[current_arg], "-r") == 0 || strcmp(argv[current_arg], "--remove_block") == 0){
            if(current_arg + 1 >= argc){
                break;
            }
            #ifdef DLL
            void (*remove_block)(table_block*, int);
            remove_block = dlsym(dll, "remove_block");
            #endif
            int index = atoi(argv[++current_arg]);
            timer_start(remove_timer);
            remove_block(tb, index);
            timer_stop(remove_timer);
            current_arg++;
        }

        else if(strcmp(argv[current_arg], "-a") == 0 || strcmp(argv[current_arg], "--alternately")){
            if(current_arg + 2 >= argc){
                break;
            }
            tb->tmp_file = argv[++current_arg];
            int count = atoi(argv[++current_arg]);
            #ifdef DLL
            int (*alloc_block)(table_block*);
            alloc_block = dlsym(dll, "alloc_block");
            void (*remove_block)(table_block*, int);
            remove_block = dlsym(dll, "remove_block");
            #endif
            int index = atoi(argv[index]);
            int i;
            timer_start(alternately_timer);
            for(i = 0; i < count ; i++){
                int index = alloc_block(tb);
                remove_block(tb, index);
            }
            timer_stop(alternately_timer);

            current_arg++;
        }
        else{
            fprintf(stderr, "Unknown command. For help just launch program without any arguments");
        }


    }


    timer_print(find_timer, "Find timer");
    timer_print(alloc_timer, "Allocating time");
    timer_print(remove_timer, "Remove timer");
    timer_print(alternately_timer, "Alternately timer");


    remove_table_block(tb);

    #ifdef DLL
    dlclose(dll);
    #endif

    return 0;

}


void help(){
    printf("You can use this commands:\n");  
    printf("<size> -s (--search_directory) <dir> <file> <tmp_file> \tsearch file in directory\n");
    printf("<size> -r (--remove_block) <index>\tremoves block of given index\n");
    printf("<size> -a (--alternately) <file> <how many times>\tcreates and remove block alternately\n");
}

void timer_print(timer* t, char* word){
    printf("\n%s\n", word);
    printf("real:  \t %lf s\n", t->time_real);
    printf("user:  \t %lf s \n", t->time_user);
    printf("system:\t %lf s\n", t->time_system);

}

void timer_reset(timer* t){
    t->time_real = 0;
    t->time_system = 0;
    t->time_user = 0;
}

void timer_start(timer* t){
    t->clock_start = times(&(t->start));
}

void timer_stop(timer* t){
    struct tms end;
    clock_t clock_end = times(&end);

    t->time_real = time_diff(t->clock_start, clock_end);
    t->time_system = time_diff(t->start.tms_stime, end.tms_stime);
    t->time_user = time_diff(t->start.tms_utime, end.tms_utime);

}

double time_diff(clock_t start, clock_t end){
    return (double) (end - start) / sysconf(_SC_CLK_TCK);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>


#include "helper.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

typedef struct image{
    int width;
    int height;
    int **value;
}image;

typedef struct filter{
    int size;
    double** value;
}filter;




filter* allocate_filter(int size);
void deallocate_filter(filter* fl);
void save_filter(filter* flr, char* path);



int main(int argc, char *argv[]){
    srand( time(NULL));
    int size = atoi(argv[1]);
    char* output_path = argv[2];

    filter* flr = allocate_filter(size);
    double sum = 0.0;
    for(int i = 0; i < flr->size; i++){
        for(int j = 0; j < flr->size; j++){;
            flr->value[i][j] = (double)rand()/(double)RAND_MAX;
            printf("%lf\n", flr->value[i][j]);
            sum += flr->value[i][j];
        }
    }
    printf("%lf\n", sum);
    for(int i = 0; i < flr->size; i++){
        for(int j = 0; j < flr->size; j++){;
            flr->value[i][j] = flr->value[i][j]/sum;
            printf("%lf\n", flr->value[i][j]);
        }
    }


    save_filter(flr, output_path);
    deallocate_filter(flr);
    //deallocate_image(input);
    return 0;

}





void save_filter(filter* flr, char* path){
    FILE* file = fopen(path, "w");

    fprintf(file, "%d\n", flr->size);

    for(int i = 0; i < flr->size; i++){
        for(int j = 0; j < flr->size; j++){
            fprintf(file, "%lf", flr->value[i][j]);
            if(j + 1 != flr->size){
                fputc(' ', file);
            }
        }
        fputc('\n', file);
    }
}


filter* allocate_filter(int size){
    filter* fl = malloc(sizeof(filter));

    fl->size = size;

    fl->value = malloc(fl->size * sizeof(double*));
    for(int i = 0; i < size; i++){
        fl->value[i] = calloc(fl->size, sizeof(double));
    }
    return fl;
}

void deallocate_filter(filter* fl){
    for(int i = 0; i < fl->size; i++){
        free(fl->value[i]);
    }
    free(fl->value);
    free(fl);
}
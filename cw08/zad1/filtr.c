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

image* input;
image* output;
filter* fltr;
int m;

image* read_image(char* path);
image* allocate_image(int width, int height);
void save_image(image* img, char* path);
void deallocate_image(image* img);
filter* read_filter(char* path);
filter* allocate_filter(int size);
void deallocate_filter(filter* fl);
static void* thread_block(void *arg);
static void* thread_interleaved(void *arg);
void formula(int x, int y);

int main(int argc, char *argv[]){
    if(argc  < 6){
        ferr("pass number of threads, method, input, filter, output");
    }
    m = atoi(argv[1]);
    char* method = argv[2];
    char* input_path = argv[3];
    char* filter_path = argv[4];
    char* output_path = argv[5];


    input = read_image(input_path);
    output = allocate_image(input->width, input->height);
    fltr = read_filter(filter_path);

    pthread_t* threads = malloc(m * sizeof(pthread_t));
    struct timeval start = get_curr_time();
    for(int k = 0; k < m; k++){
        int* val = malloc(sizeof(int));
        *val = k;
        if(strcmp(method, "block") == 0){
            pthread_create(threads + k, NULL, &thread_block, val);
        } else if(strcmp(method, "interleaved") == 0){
            pthread_create(threads + k, NULL, &thread_interleaved, val);
        } else {
            ferr("wrong method name");
        }
    }
    for(int k = 0; k < m; k++){
        long* time_result;
        pthread_join(*(threads + k), (void *)&time_result);
        printf("thread ID: %d\ttime: %ldus\n", k+1, *time_result);
    }
    printf("total process   time: %ldus\n", time_diff(start, get_curr_time()));
    save_image(output, output_path);
    deallocate_filter(fltr);
    //deallocate_image(input);
    deallocate_image(output);
    return 0;

}

static void* thread_block(void *arg){
    struct timeval start = get_curr_time();

    int k = (*(int*) arg);

    int from = k * ceil(input->width / m);
    int to = (k+1) * ceil(input->width / m);
    for(int x = from; x < to; x++){
        for(int y = 0; y < input->height; y++){
            formula(y, x);
        }
    }

    long* time_result = malloc(sizeof(long));
    *time_result = time_diff(start, get_curr_time());
    pthread_exit(time_result);
}

static void* thread_interleaved(void *arg){
    struct timeval start = get_curr_time();

    int k = (*(int*) arg);
    for(int x = k; x < input->width; x += m){
        for(int y = 0; y < input->height; y++){
            formula(y, x);
        }
    }

    long* time_result = malloc(sizeof(long));
    *time_result = time_diff(start, get_curr_time());
    pthread_exit(time_result);
}

void formula(int y, int x){
    int c = fltr->size;
    double s = 0;
    for(int i = 0; i < c; i++){
        for(int j = 0; j < c; j++){
            int y1 = MAX(0, y - (int)ceil(c/2) + i);
            int x1 = MAX(0, x - (int)ceil(c/2) + j);
            if(y1 >= input->height) y1 = input->height - 1;
            if(x1 >= input->width) x1 = input->width - 1;
            s += input->value[y1][x1] * fltr->value[i][j]; 
        }
    }
    output->value[y][x] = round(s);
}

image* read_image(char* path){
    FILE* file = fopen(path, "r");
    if(file == NULL) ferrno();

    int width;
    int height;

    fscanf(file, "P2 %d %d 255", &width, &height);
    image* img = allocate_image(width, height);

    for(int i = 0; i < img->height; i++){
        for(int j = 0; j < img->width; j++){
            fscanf(file, "%d", &img->value[i][j]);
        }
    }

    fclose(file);
    return img;

}

image* allocate_image(int width, int height){
    image* img = malloc(sizeof(image));

    img->width = width;
    img->height = height;

    img->value = malloc(img->height*sizeof(int*));
    for(int i = 0; i < height; i++){
        img->value[i] = calloc(img->width, sizeof(int));
    }
    return img;
}

void deallocate_image(image* img){
    for(int i = 0; i < img->height; i++){
        free(img->value[i]);
    }
    free(img->value);
    free(img);
}

void save_image(image* img, char* path){
    FILE* file = fopen(path, "w");

    fprintf(file, "P2\n%d %d\n255\n", img->width, img->height);

    for(int i = 0; i < img->height; i++){
        for(int j = 0; j < img->width; j++){
            fprintf(file, "%d", img->value[i][j]);
            if(j + 1 != img->width){
                fputc(' ', file);
            }
        }
        fputc('\n', file);
    }
}

filter* read_filter(char* path){
    FILE* file = fopen(path, "r");
    if(file == NULL) ferrno();

    int size;

    fscanf(file, "%d", &size);
    filter* fl = allocate_filter(size);
    long sum = 0;
    for(int i = 0; i < fl->size; i++){
        for(int j = 0; j < fl->size; j++){
            fscanf(file, "%lf", &fl->value[i][j]);
            sum +=fl->value[i][j];
        }
    }


    fclose(file);
    return fl;
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
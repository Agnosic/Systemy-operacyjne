#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

void help();


int main(int argc, char* argv[]){
    if(argc !=  5){
        help();
        exit(1);
    }   

    srand(time(NULL));

    char* fname = argv[1];
    int pmin = atoi(argv[2]);
    int pmax = atoi(argv[3]);
    int bytes = atoi(argv[4]);

    char* text = calloc(bytes+1, sizeof(char));

    for(int i = 0; i < bytes; i++){
        text[i] = 'x';
    }
    text[bytes] = '\0';
    char* date = malloc(25);

    while(1){
        int interval = rand()%(abs(pmax-pmin) + 1) + pmin;
        FILE* file = fopen(fname, "a");

        time_t t= time(NULL);
        strftime(date, 25, "_%Y-%m-%d_%H-%M-%S", localtime(&t));
        fprintf(file, "%d %s %s\n\n", interval, date, text);
        fclose(file);
        sleep(interval);
    }
    free(date);

    return 0;
}

void help(){
    fprintf(stderr, "\n .\\tester <plik> <pmin> <pmax> <bytes>\n");
}
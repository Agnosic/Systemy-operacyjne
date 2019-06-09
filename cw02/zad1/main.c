#include <stdio.h>
#include <unistd.h>
#include <sys/times.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/stat.h>
#include <fcntl.h>

void generate(char* fileName, int size, int count);
void sys_sort(char* fileName, int size, int count);
void lib_sort(char* fileName, int size, int count);
void sys_copy(char* fromFileName, char* toFileName, int size, int count);
void lib_copy(char* fromFileName, char* toFileName, int size, int count);
void help();
double time_diff(clock_t start, clock_t stop);

int main(int argc, char* argv[]){
    if(argc == 1){
        help();
        exit(0);
    }
    int arg = 1;
    while(arg < argc){
        struct tms start;
        clock_t real_start = times(&start);
        if(strcmp(argv[arg], "generate") == 0){
            if(arg + 3 >= argc){
                fprintf(stderr, "Brakuje kilku argumentow\n");
                exit(1);
            }
            char* fileName = argv[++arg];
            int size = atoi(argv[++arg]);
            int count = atoi(argv[++arg]);

            generate(fileName, size, count);
        }
        else if(strcmp(argv[arg], "sort") == 0){
            if(arg + 4 >= argc){
                fprintf(stderr, "Brakuje kilku argumentow\n");
                exit(1);
            }
            char* fileName = argv[++arg];
            int size = atoi(argv[++arg]);
            int count = atoi(argv[++arg]);
            char* method = argv[++arg];
            if(strcmp(method, "sys") == 0){
                sys_sort(fileName, size, count);
            }
            else if(strcmp(method, "lib") == 0){
                lib_sort(fileName, size, count);
            }
            else{
                fprintf(stderr, "Must be lib or sys");
                exit(1);
            }

        }
        else if(strcmp(argv[arg], "copy") == 0){
            if(arg + 5 >= argc){
                fprintf(stderr, "Brakuje kilku argumentow\n");
                exit(1);
            }
            char* fromFile = argv[++arg];
            char* toFile = argv[++arg];
            int size = atoi(argv[++arg]);
            int count = atoi(argv[++arg]);
            char* method = argv[++arg];
            if(strcmp(method, "sys") == 0){
                sys_copy(fromFile, toFile, size, count);
            }
            else if(strcmp(method, "lib") == 0){
                lib_copy(fromFile, toFile, size, count);
            }
            else{
                fprintf(stderr, "Must be lib or sys\n");
                exit(1);
            }
        }
        else{
            fprintf(stderr, "Wrong command\n");
            exit(1);
        }

        arg++;
        struct tms stop;
        clock_t real_stop = times(&stop);
        double real_time = time_diff(real_start, real_stop);
        double user_time = time_diff(start.tms_utime, stop.tms_utime);
        double sys_time = time_diff(start.tms_stime, stop.tms_stime);
        for(int i = 0; i < argc; i++){
            printf("%s ", argv[i]);
        }
        printf("\nReal time:  \t%lf\n", real_time);
        printf("User time:  \t%lf\n", user_time);
        printf("System time:\t%lf\n\n", sys_time);


    }


}

void help(){
    printf("You can use this commands:\n");  
    printf("generate <file> <size> <count>             \t powinno losowo generować <size> rekordów o długości <count> bajtów do pliku <file>,\n\n");
    printf("sort <file> <size> <count> sys/lib         \t powinien sortować rekordy w pliku dane przy użyciu funkcji systemowych lub biblioteki C, zakładając że zawiera on <size> rekordów wielkości <count> bajtów\n\n");
    printf("copy <file1> <file2> <size> <count> sys/lib\t powinno skopiować <size> rekordów <file1> do <file2> za pomocą funkcji bibliotecznych C lub funckji systemowych z wykorzystaniem bufora <count> bajtów\n\n");
}

void generate(char* fileName, int size, int count){
    FILE* file = fopen(fileName, "w");
    if(!file){
        fprintf(stderr, "Nie udalo sie stworzyc plik\n");
        exit(1);
    }

    FILE* file_random = fopen("/dev/urandom", "r");
    if(!file_random){
        fprintf(stderr, "Nie udalo sie odczytac pliku z /dev/random\n");
        exit(1);
    }


    
    unsigned char* buffor = calloc(count * size, sizeof(char));
    if(!buffor){
        fprintf(stderr, "Nie udalo sie zalokowac pamieci do buforow\n");
        exit(1);
    }
    fread(buffor, sizeof(char), count * size, file_random);
    fwrite(buffor, sizeof(char), count * size, file);
    if(fclose(file) == 0){
        perror(fileName);
    }
    if(fclose(file_random) == 0){
        perror("/dev/random");
    }

}

void lib_sort(char* fileName, int size, int count){
    FILE* file = fopen(fileName, "r+");
    if(!file){
        fprintf(stderr, "Nie udalo sie otworzyc plik\n");
        exit(1);
    }

    unsigned char* buffor1 = calloc(count, sizeof(char));
    unsigned char* buffor2 = calloc(count, sizeof(char));

    if(!buffor1 || !buffor2){
        fprintf(stderr, "Nie udalo sie zalokowac pamieci do buforow\n");
        exit(1);
    }

    for(int i = 0; i < size - 1; i++){
        fseek(file, i*size, SEEK_SET);
        fread(buffor1, sizeof(char), count, file);
        unsigned char min = buffor1[0];
        int min_rec = i;

        for(int j = i+1; j < count; j++){
            fseek(file, j*size, SEEK_SET);
            unsigned char curr = (unsigned char) fgetc(file);
            if(curr < min){
                min = curr;
                min_rec = j;
            }

        }

        fseek(file, min_rec * count, SEEK_SET);

        fread(buffor2, sizeof(char), count, file);

        fseek(file, min_rec * count, SEEK_SET);

        fwrite(buffor1, sizeof(char), count, file);

        fseek(file, i * count, SEEK_SET);

        fwrite(buffor2, sizeof(char), count, file);
    }
    free(buffor1);
    free(buffor2);
    if(fclose(file) == 0){
        perror(fileName);
    }


}

void sys_sort(char* fileName, int size, int count){
    int file = open(fileName, O_RDWR);
    if(!file){
        fprintf(stderr, "Nie udalo sie otworzyc plik\n");
        exit(1);
    }

    unsigned char* buffor1 = calloc(count, sizeof(char));
    unsigned char* buffor2 = calloc(count, sizeof(char));

    if(!buffor1 || !buffor2){
        fprintf(stderr, "Nie udalo sie zalokowac pamieci do buforow\n");
        exit(1);
    }

    for(int i = 0; i < size - 1; i++){
        lseek(file, i*size, SEEK_SET);
        read(file, buffor1, count * sizeof(char));
        unsigned char min = buffor1[0];
        int min_rec = i;

        for(int j = i+1; j < count; j++){
            lseek(file, j*size, SEEK_SET);
            unsigned char curr;
            read(file, &curr, sizeof(char));
            if(curr < min){
                min = curr;
                min_rec = j;
            }

        }

        lseek(file, min_rec * count, SEEK_SET);

        read(file, buffor2, sizeof(char) * count);

        lseek(file, min_rec * count, SEEK_SET);

        write(file, buffor1, sizeof(char) * count);

        lseek(file, i * count, SEEK_SET);

        write(file, buffor2, sizeof(char) * count);
    }
    free(buffor1);
    free(buffor2);
    if(close(file) == 0){
        perror(fileName);
    }

}

void sys_copy(char* fromFileName, char* toFileName, int size, int count){
    int fromFile = open(fromFileName, O_RDONLY);
    if(!fromFile){
        fprintf(stderr, "Nie udalo sie otworzyc plik\n");
        exit(1);
    }
    int toFile = open(toFileName, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    if(!toFile){
        fprintf(stderr, "Nie udalo sie stworzyc plik\n");
        exit(1);
    }
    unsigned char* buffor = calloc(count * size, sizeof(char));
    if(!buffor){
        fprintf(stderr, "Nie udalo sie zalokowac pamieci do buforow\n");
        exit(1);
    }
    read(fromFile, buffor, sizeof(char) * size * count);
    write(toFile, buffor, sizeof(char) * size * count);
    

    if(close(fromFile) == 0){
        perror(fromFileName);
    }
    if(close(toFile) == 0){
        perror(toFileName);
    }
}



void lib_copy(char* fromFileName, char* toFileName, int size, int count){
    FILE* fromFile = fopen(fromFileName, "r");
    if(!fromFile){
        fprintf(stderr, "Nie udalo sie otworzyc plik\n");
        exit(1);
    }
    FILE* toFile = fopen(toFileName, "w");
    if(!toFile){
        fprintf(stderr, "Nie udalo sie stworzyc plik\n");
        exit(1);
    }
    unsigned char* buffor = calloc(count * size, sizeof(char));
    if(!buffor){
        fprintf(stderr, "Nie udalo sie zalokowac pamieci do buforow\n");
        exit(1);
    }
    fread(buffor, sizeof(char), size * count, fromFile);
    fwrite(buffor, sizeof(char), size * count, toFile);


    if(fclose(fromFile) == 0){
        perror(fromFileName);
    }
    if(fclose(toFile) == 0){
        perror(toFileName);
    }

}

double time_diff(clock_t start, clock_t stop){
    return (double) (stop - start) / sysconf(_SC_CLK_TCK);
}
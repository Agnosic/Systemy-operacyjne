#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "blocks.h"


table_block* create_table(int size){
    if(size < 0){
        fprintf(stderr, "Size must be positive");
        exit(1);
    }
    table_block * tb = calloc(1, sizeof(table_block));

    char** table = (char**)calloc(size, sizeof(char*));

    tb->size = size;
    tb->table = table;

    return tb;

}


//////////////////////////////////////////////////////

void find(table_block* tb){
    if(tb == NULL || tb->table == NULL){
        fprintf(stderr, "Block does not exist");
        exit(1);
    }

    if(tb->dir == NULL){
        fprintf(stderr, "dir is not assigned");
        exit(1);
    }

    if(tb->file == NULL){
        fprintf(stderr, "file is not assigned");
        exit(1);
    }

    if(tb->tmp_file == NULL){
        fprintf(stderr, "tmp file is not assigned");
        exit(1);
    }

    char* order = (char*)calloc(1, strlen(tb->file) + strlen(tb->tmp_file) + strlen(tb->dir) + 20);

    sprintf(order, "find \"%s\" -name \"%s\" > \"%s\"", tb->dir, tb->file, tb->tmp_file);

    system(order);

    free(order);

}


//////////////////////////////////////////////////////
int alloc_block(table_block* tb){
    if(tb == NULL || tb->table == NULL){
        fprintf(stderr, "Block does not exist");
        exit(1);
    }
    if(tb->tmp_file == NULL){
        fprintf(stderr, "tmp_file is not assigned");
        exit(1);
    }
    int index = 0;
    while(index < tb->size && tb->table[index] != NULL){
        index++;
    }

    if(tb->table[index] != NULL){
        fprintf(stderr, "table is full");
        exit(1);
    }

    FILE* tmp_file = fopen(tb->tmp_file, "r");
    
    if(tmp_file == NULL){
        fprintf(stderr, "Cannot open a file");
        exit(1);
    }

    fseek(tmp_file, 0, SEEK_END);
    long sz = ftell(tmp_file);
    fseek(tmp_file, 0, SEEK_SET);


    tb->table[index] = calloc(1, sz);

    fread(tb->table[index], sz, 1, tmp_file);

    fclose(tmp_file);

    return index;


}

//////////////////////////////////////////////////////
void remove_block(table_block* tb, int index){
    if(tb == NULL || tb->table == NULL){
        fprintf(stderr, "Block does not exist");
        exit(1);
    }
    if (index < 0){
        fprintf(stderr, "Index must be positive");
        exit(1);
    }
    if(index >= tb->size){
        fprintf(stderr, "Index is bigger than block table size");
        exit(1);
    }
    if(tb->table[index] == NULL){
        fprintf(stderr, "Block was already removed");
        exit(1);
    }

    free(tb->table[index]);
    tb->table[index] = NULL;
}
//////////////////////////////////////////////////////
void remove_table_block(table_block* tb){
    if(tb == NULL){
        fprintf(stderr, "table block does not exist");
        exit(1);
    }
    if(tb->table != NULL){
        free(tb->table);
    }

    free(tb);
}

////////////////////////////////////////////////////////



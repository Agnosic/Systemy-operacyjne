#ifndef ZAD_LIBRARY
#define ZAD_LIBRARY

typedef struct{
    char** table;
    int size;
    char* dir;
    char* file;
    char* tmp_file;
}table_block;

extern void find(table_block* tb);
extern table_block* create_table(int size);
extern void remove_block(table_block* tb, int index);
extern int alloc_block(table_block* tb);
extern void remove_table_block(table_block* tb);

#endif //ZAD_LIBRARY
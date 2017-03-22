#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BLOCK_SIZE 8

#include "hash.h"

char *hash(FILE *f) {
    
	char *hash_val = (char*) malloc(BLOCK_SIZE + 1);
    
    if (hash_val == NULL)
    {
        printf("Out of memory!\n");
        exit(1);
    }
    
    hash_val[BLOCK_SIZE + 1] = 0;
    
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        hash_val[i] = '0';
    }
    
    // read into buffer in one operation because the I/O read is slow and expensive
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *content = malloc(fsize + 1);
    fread(content, fsize, 1, f);

    int error = fclose(f);
    
    if (error != 0) {
        printf("%s\n", "error closing the file");
        exit(1);
    }
    
    content[fsize] = 0;
    
    for (long i = 0; i < fsize; i++) {
        hash_val[i % BLOCK_SIZE] = content[i] ^ hash_val[i];
    }
    
    return hash_val;
}

#include "mass_storage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "errors.h"

#define BLOCK_SIZE 1024

FILE *fp = 0;

int open_mass_storage(char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Could not open file %s\n", filename);
        return ERROR_FILE_NOT_FOUND;
    }
    fp = file;
    return 0;
}

int close_mass_storage() {
    fclose(fp);
    return 0;
}

int read_block_from_mass_storage(int block_num, uint8_t *buf) {
    if (fp == 0) {
        printf("Mass storage not open\n");
        return ERROR_FILE_NOT_OPEN;
    }
    fseek(fp, block_num * BLOCK_SIZE, SEEK_SET);
    int read = fread(buf, 1, BLOCK_SIZE, fp);
    if (read < BLOCK_SIZE) {
        printf("Could not read block %i\n", block_num);
        return ERROR_BLOCK_CANNOT_BE_READ;
    }
    return 0;
}
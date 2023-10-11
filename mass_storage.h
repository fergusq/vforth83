#ifndef _MASS_STORAGE_H
#define _MASS_STORAGE_H

#include <stdint.h>

#include "errors.h"

#define BLOCK_SIZE 1024

int open_mass_storage(char *filename);

int close_mass_storage();

int read_block_from_mass_storage(int block_num, uint8_t *buf);

#endif
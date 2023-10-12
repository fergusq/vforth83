#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <glob.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "dos.h"
#include "memory.h"
#include "util.h"

uint8_t ZERO_FLAG;
uint8_t *DTA;

FCB *xfcb_to_fcb(XFCB *xfcb) {
    return (FCB*) (xfcb - FCB_EXTENDED_FCB_FLAG_OFFSET);
}

uint8_t is_xfcb(uint8_t *pointer) {
    return *pointer == 0xFF;
}

FCB *ensure_fcb(uint8_t *pointer) {
    if (is_xfcb(pointer)) {
        return xfcb_to_fcb((XFCB*) pointer);
    } else {
        return (FCB*) pointer;
    }
}

void empty_fcb(FCB *fcb) {
    memset(fcb, 0, sizeof(*fcb));
}

void set_filename(uint8_t *fcb_file_name, uint8_t *fcb_file_extension, char *filename) {
    char *dot = strchr(filename, '.');
    memset(fcb_file_name, ' ', 8);
    memset(fcb_file_extension, ' ', 3);
    if (dot == NULL) {
        int length = strlen(filename);
        if (length <= 8) {
            memcpy(fcb_file_name, filename, strlen(filename));
        } else {
            memcpy(fcb_file_name, filename, 5);
            fcb_file_name[5] = '~';
            uint8_t hash = pearson_hash(filename);
            fcb_file_name[6] = '0' + hash / 16;
            fcb_file_name[7] = '0' + hash % 16;
        }
        memset(fcb_file_extension, ' ', 3);
    } else {
        int length = dot - filename;
        int extension_length = strlen(dot + 1);
        if (length <= 8 && extension_length <= 3) {
            memcpy(fcb_file_name, filename, dot - filename);
        } else {
            memcpy(fcb_file_name, filename, 5);
            fcb_file_name[5] = '~';
            uint8_t hash = pearson_hash(filename);
            fcb_file_name[6] = '0' + hash / 16;
            fcb_file_name[7] = '0' + hash % 16;
        }
        if (extension_length <= 3) {
            memcpy(fcb_file_extension, dot + 1, extension_length);
        } else {
            memcpy(fcb_file_extension, dot + 1, 1);
            fcb_file_extension[1] = '~';
            fcb_file_extension[2] = '0' + extension_length % 60;
        }
    }
}

char *get_filename(FCB *fcb) {
    char *filename = malloc(13);
    memcpy(filename, fcb->file_name, 8);
    if (fcb->file_extension[0] == ' ') {
        filename[8] = 0;
        return filename;
    }
    filename[8] = '.';
    memcpy(filename + 9, fcb->file_extension, 3);
    filename[12] = 0;
    return filename;
}

char *get_real_filename(char *dos_filename) {
    if (dos_filename[5] == '~') {
        char pattern[11];
        memcpy(pattern, dos_filename, 5);
        pattern[5] = '*';
        if (dos_filename[8] == 0) {
            pattern[6] = '.';
        }
        pattern[6] = '.';
        pattern[7] = dos_filename[9];
        if (dos_filename[10] == '~') {
            pattern[8] = '*';
            pattern[9] = 0;
        } else {
            pattern[8] = dos_filename[10];
            pattern[9] = dos_filename[11];
            pattern[10] = 0;
        }
        glob_t globbuf;
        glob(pattern, GLOB_NOESCAPE, NULL, &globbuf);
        for (int i = 0; i < globbuf.gl_pathc; i++) {
            char *filename2 = globbuf.gl_pathv[i];
            char dos_filename2[13];
            dos_filename2[12] = 0;
            set_filename(dos_filename2, dos_filename2 + 9, filename2);
            if (dos_filename2[9] != ' ') {
                dos_filename2[8] = '.';
            }
            if (strcmp(dos_filename, dos_filename2) == 0) {
                char *filename = strdup(filename2);
                globfree(&globbuf);
                return filename;
            }
        }
        return NULL;
    } else {
        return strdup(dos_filename);
    }
}

uint16_t get_file_date(char *filename) {
    struct stat attr;
    stat(filename, &attr);
    struct tm *time = localtime(&attr.st_mtime);
    int year = time->tm_year - 1980;
    int month = time->tm_mon + 1;
    int day = time->tm_mday;
    return (year << 9) | (month << 5) | day;
}

uint16_t get_file_time(char *filename) {
    struct stat attr;
    stat(filename, &attr);
    struct tm *time = localtime(&attr.st_mtime);
    int hours = time->tm_hour;
    int minutes = time->tm_min;
    int seconds = time->tm_sec / 2;
    return (hours << 11) | (minutes << 5) | seconds;
}

uint8_t function_00H_Terminate_Process(Memory *memory, uint16_t unused) {
    exit(0);
}

uint8_t function_01H_Character_Input_With_Echo(Memory *memory, uint16_t unused) {
    return getchar();
}

uint8_t function_02H_Character_Output(Memory *memory, uint16_t character) {
    putchar(character);
    return 0;
}

uint8_t function_05H_Print_Character(Memory *memory, uint16_t character) {
    putchar(character);
    return 0;
}

/*
uint8_t function_06H_Direct_Console_IO(Memory *memory, uint16_t character) {
    if (character == 0xFF) {
        // Should be non-blocking
        ZERO_FLAG = 0;
        return getchar();
    } else {
        putchar(character);
        return 0;
    }
}
*/

uint8_t function_08H_Character_Input_Without_Echo(Memory *memory, uint16_t character) {
    return getchar();
}

/*** FILE IO ***/

#define MAX_OPEN_FILES 256

FILE *OPEN_FILES[MAX_OPEN_FILES] = {0};

uint8_t function_0FH_Open_File(Memory *memory, uint16_t fcb_pointer) {
    FCB *fcb = ensure_fcb(memory_at8(memory, fcb_pointer));
    if (fcb->reserved != 0) { // Check if this is already associated with a file handle
        return 0xFF;
    }
    char *filename = get_filename(fcb);
    char *real_filename = get_real_filename(filename);
    FILE *file = fopen(real_filename, "rw");
    free(filename);
    if (file == NULL) {
        return 0xFF;
    }
    // Find a free slot for the file
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (OPEN_FILES[i] == NULL) {
            OPEN_FILES[i] = file;
            fcb->reserved = i + 1;
            fcb->record_size = 128;
            fcb->drive_identifier = 0;
            fcb->current_block_number = 0;
            fcb->current_record_number = 0;
            fcb->file_size = fseek(file, 0, SEEK_END);
            fcb->data_stamp = get_file_date(real_filename);
            fcb->time_stamp = get_file_time(real_filename);
            free(real_filename);
            return 0x00;
        }
    }
    fclose(file);
    free(real_filename);
    return 0xFF;
}

uint8_t function_10H_Close_File(Memory *memory, uint16_t fcb_pointer) {
    FCB *fcb = ensure_fcb(memory_at8(memory, fcb_pointer));
    if (fcb->reserved == 0) { // Check if this is already associated with a file handle
        return 0xFF;
    }
    fclose(OPEN_FILES[fcb->reserved - 1]);
    OPEN_FILES[fcb->reserved - 1] = NULL;
    fcb->reserved = 0;
    return 0x00;
}

glob_t globbuf;
int glob_offset = 0;
uint16_t previous_glob_fcb = 0;

uint8_t function_11H_Find_First_File(Memory *memory, uint16_t fcb_pointer) {
    uint8_t xfcb = is_xfcb(memory_at8(memory, fcb_pointer));
    FCB *fcb = ensure_fcb(memory_at8(memory, fcb_pointer));
    char *filename = get_filename(fcb);
    if (glob_offset > 0) globfree(&globbuf);
    glob(filename, GLOB_NOESCAPE, NULL, &globbuf);
    free(filename);
    glob_offset = 0;
    FCB *result = xfcb ? DTA - FCB_EXTENDED_FCB_FLAG_OFFSET : DTA;
    empty_fcb(result);
    set_filename(&result->file_name, &result->file_extension, globbuf.gl_pathv[glob_offset]);
    glob_offset += 1;
    previous_glob_fcb = fcb_pointer;
    return 0x00;
}

uint8_t function_12H_Find_Next_File(Memory *memory, uint16_t fcb_pointer) {
    if (fcb_pointer != previous_glob_fcb) {
        return 0xFF;
    }
    if (glob_offset >= globbuf.gl_pathc) {
        return 0xFF;
    }
    uint8_t xfcb = is_xfcb(memory_at8(memory, fcb_pointer));
    FCB *result = xfcb ? DTA - FCB_EXTENDED_FCB_FLAG_OFFSET : DTA;
    empty_fcb(result);
    set_filename(&result->file_name, &result->file_extension, globbuf.gl_pathv[glob_offset]);
    glob_offset += 1;
    return 0x00;
}

uint8_t function_13H_Delete_File(Memory *memory, uint16_t fcb_pointer) {
    FCB *fcb = ensure_fcb(memory_at8(memory, fcb_pointer));
    char *filename = get_filename(fcb);
    char *real_filename = get_real_filename(filename);
    glob_t globbuf;
    glob(real_filename, GLOB_NOESCAPE, NULL, &globbuf);
    free(filename);
    free(real_filename);
    if (globbuf.gl_pathc == 0) {
        return 0xFF;
    }
    int result = 0x00;
    for (int i = 0; i < globbuf.gl_pathc; i++) {
        if (remove(globbuf.gl_pathv[i]) == 0) {
            result = 0xFF;
        }
    }
    globfree(&globbuf);
    return result;
}

uint8_t function_14H_Sequential_Read(Memory *memory, uint16_t fcb_pointer) {
    FCB *fcb = ensure_fcb(memory_at8(memory, fcb_pointer));
    if (fcb->reserved == 0) { // Check if this is already associated with a file handle
        return 0xFF;
    }
    FILE *file = OPEN_FILES[fcb->reserved - 1];
    if (file == NULL) {
        return 0xFF;
    }
    size_t block_size = fcb->record_size * 128;
    if (fseek(file, fcb->current_block_number * block_size + fcb->current_record_number * fcb->record_size, SEEK_SET) != 0) {
        return 0xFF;
    }
    size_t n = fcb->record_size;
    size_t ret_code = fread(DTA, 1, n, file);
    if (ret_code == 0) {
        return 0x01;
    } else if (ret_code < n) {
        fcb->current_record_number += 1;
        if (fcb->current_record_number == 128) {
            fcb->current_block_number += 1;
            fcb->current_record_number = 0;
        }
        size_t padding = n - ret_code;
        memset(DTA + ret_code, 0, padding);
        return 0x03;
    } else {
        fcb->current_record_number += 1;
        if (fcb->current_record_number == 128) {
            fcb->current_block_number += 1;
            fcb->current_record_number = 0;
        }
        return 0x00;
    }
}

uint8_t function_15H_Sequential_Write(Memory *memory, uint16_t fcb_pointer) {
    FCB *fcb = ensure_fcb(memory_at8(memory, fcb_pointer));
    if (fcb->reserved == 0) { // Check if this is already associated with a file handle
        return 0xFF;
    }
    FILE *file = OPEN_FILES[fcb->reserved - 1];
    if (file == NULL) {
        return 0xFF;
    }
    size_t block_size = fcb->record_size * 128;
    if (fseek(file, fcb->current_block_number * block_size + fcb->current_record_number * fcb->record_size, SEEK_SET) != 0) {
        return 0xFF;
    }
    size_t n = fcb->record_size;
    size_t ret_code = fwrite(DTA, 1, n, file);
    if (ret_code < n) {
        return 0x01;
    } else {
        fcb->current_record_number += 1;
        if (fcb->current_record_number == 128) {
            fcb->current_block_number += 1;
            fcb->current_record_number = 0;
        }
        return 0x00;
    }
}

uint8_t function_16H_Create_File_With_FCB(Memory *memory, uint16_t fcb_pointer) {
    FCB *fcb = ensure_fcb(memory_at8(memory, fcb_pointer));
    if (fcb->reserved != 0) { // Check if this is already associated with a file handle
        return 0xFF;
    }
    char *filename = get_filename(fcb);
    char *real_filename = get_real_filename(filename);
    FILE *file = fopen(real_filename, "w");
    free(real_filename);
    free(filename);
    if (file == NULL) {
        return 0xFF;
    }
    // Find a free slot for the file
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (OPEN_FILES[i] == NULL) {
            OPEN_FILES[i] = file;
            fcb->reserved = i + 1;
            fcb->record_size = 128;
            fcb->drive_identifier = 0;
            fcb->current_block_number = 0;
            fcb->file_size = 0;
            return 0x00;
        }
    }
    fclose(file);
    return 0xFF;
}

uint8_t function_19H_Get_Default_Drive(Memory *memory, uint16_t unused) {
    return 0;
}

uint8_t function_1AH_Set_DTA_Address(Memory *memory, uint16_t dta) {
    DTA = memory_at8(memory, dta);
    return 0;
}

uint8_t function_21H_Random_Read(Memory *memory, uint16_t fcb_pointer) {
    FCB *fcb = ensure_fcb(memory_at8(memory, fcb_pointer));
    if (fcb->reserved == 0) { // Check if this is already associated with a file handle
        return 0xFF;
    }
    FILE *file = OPEN_FILES[fcb->reserved - 1];
    if (file == NULL) {
        return 0xFF;
    }
    size_t block_size = fcb->record_size * 128;
    fcb->current_block_number = fcb->random_record_number / 128;
    fcb->current_record_number = fcb->random_record_number % 128;
    if (fseek(file, fcb->current_block_number * block_size + fcb->current_record_number * fcb->record_size, SEEK_SET) != 0) {
        return 0xFF;
    }
    size_t n = fcb->record_size;
    size_t ret_code = fread(DTA, 1, n, file);
    if (ret_code == 0) {
        return 0x01;
    } else if (ret_code < n) {
        fcb->current_record_number += 1;
        if (fcb->current_record_number == 128) {
            fcb->current_block_number += 1;
            fcb->current_record_number = 0;
        }
        size_t padding = n - ret_code;
        memset(DTA + ret_code, 0, padding);
        return 0x03;
    } else {
        fcb->current_record_number += 1;
        if (fcb->current_record_number == 128) {
            fcb->current_block_number += 1;
            fcb->current_record_number = 0;
        }
        return 0x00;
    }
}

uint8_t function_22H_Random_Write(Memory *memory, uint16_t fcb_pointer) {
    FCB *fcb = ensure_fcb(memory_at8(memory, fcb_pointer));
    if (fcb->reserved == 0) { // Check if this is already associated with a file handle
        return 0xFF;
    }
    FILE *file = OPEN_FILES[fcb->reserved - 1];
    if (file == NULL) {
        return 0xFF;
    }
    size_t block_size = fcb->record_size * 128;
    fcb->current_block_number = fcb->random_record_number / 128;
    fcb->current_record_number = fcb->random_record_number % 128;
    if (fseek(file, fcb->current_block_number * block_size + fcb->current_record_number * fcb->record_size, SEEK_SET) != 0) {
        return 0xFF;
    }
    size_t n = fcb->record_size;
    size_t ret_code = fwrite(DTA, 1, n, file);
    if (ret_code < n) {
        return 0x01;
    } else {
        fcb->current_record_number += 1;
        if (fcb->current_record_number == 128) {
            fcb->current_block_number += 1;
            fcb->current_record_number = 0;
        }
        return 0x00;
    }
}

uint8_t function_23H_Get_File_Size(Memory *memory, uint16_t fcb_pointer) {
    FCB *fcb = ensure_fcb(memory_at8(memory, fcb_pointer));
    if (fcb->reserved != 0) { // Check if this is already associated with a file handle
        return 0xFF;
    }
    char *filename = get_filename(fcb);
    char *real_filename = get_real_filename(filename);
    FILE *file = fopen(real_filename, "w");
    free(real_filename);
    free(filename);
    if (file == NULL) {
        return 0xFF;
    }
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    return size / fcb->record_size;
}

SysCallFunction DOS_SYSCALLS[] = {
    [0x00] = function_00H_Terminate_Process,
    [0x01] = function_01H_Character_Input_With_Echo,
    [0x02] = function_02H_Character_Output,
    [0x05] = function_05H_Print_Character,
    [0x08] = function_08H_Character_Input_Without_Echo,
    [0x0F] = function_0FH_Open_File,
    [0x10] = function_10H_Close_File,
    [0x11] = function_11H_Find_First_File,
    [0x12] = function_12H_Find_Next_File,
    [0x13] = function_13H_Delete_File,
    [0x14] = function_14H_Sequential_Read,
    [0x15] = function_15H_Sequential_Write,
    [0x16] = function_16H_Create_File_With_FCB,
    [0x19] = function_19H_Get_Default_Drive,
    [0x1A] = function_1AH_Set_DTA_Address,
    [0x21] = function_21H_Random_Read,
    [0x22] = function_22H_Random_Write,
    [0x23] = function_23H_Get_File_Size,
};
#ifndef DOS_H
#define DOS_H

#include <stdint.h>

#include "memory.h"

// File Control Block

#define FCB_EXTENDED_FCB_FLAG_OFFSET -0x7
#define FCB_FILE_ATTRIBUTE_BYTE -0x1
#define FCB_DRIVE_IDENTIFIER_OFFSET 0x0
#define FCB_FILE_NAME_OFFSET 0x1
#define FCB_FILE_EXTENSION_OFFSET 0x9
#define FCB_FILE_CURRENT_BLOCK_NUMBER_OFFSET 0xC
#define FCB_FILE_RECORD_SIZE_OFFSET 0xE
#define FCB_FILE_FILE_SIZE_OFFSET 0x10
#define FCB_FILE_DATA_STAMP_OFFSET 0x14
#define FCB_FILE_TIME_STAMP_OFFSET 0x16
#define FCB_FILE_CURRENT_RECORD_NUMBER_OFFSET 0x20
#define FCB_FILE_RANDOM_RECORD_NUMBER_OFFSET 0x21

typedef struct _FCB {
    uint8_t drive_identifier;
    uint8_t file_name[8];
    uint8_t file_extension[3];
    uint16_t current_block_number;
    uint16_t record_size;
    uint32_t file_size;
    uint16_t data_stamp;
    uint16_t time_stamp;
    uint8_t reserved[8];
    uint8_t current_record_number;
    uint16_t random_record_number;
} __attribute__((packed)) FCB;

typedef struct _XFCB {
    uint8_t extended_fcb_flag;
    uint8_t reserved1[5];
    uint8_t file_attribute_byte;
    uint8_t drive_identifier;
    uint8_t file_name[8];
    uint8_t file_extension[3];
    uint16_t current_block_number;
    uint16_t record_size;
    uint32_t file_size;
    uint16_t data_stamp;
    uint16_t time_stamp;
    uint8_t reserved2[8];
    uint8_t current_record_number;
    uint16_t random_record_number;
} __attribute__((packed)) XFCB;

FCB *xfcb_to_fcb(XFCB *xfcb);
uint8_t is_xfcb(uint8_t *pointer);
FCB *ensure_fcb(uint8_t *pointer);
void empty_fcb(FCB *fcb);
void set_filename(uint8_t *fcb_file_name, uint8_t *fcb_file_extension, char *filename);

// DOS System Calls (INT 21h)

// Process control

uint8_t function_00H_Terminate_Process(Memory *memory, uint16_t unused);

// Console I/O

uint8_t function_01H_Character_Input_With_Echo(Memory *memory, uint16_t unused);
uint8_t function_02H_Character_Output(Memory *memory, uint16_t character);
//uint8_t function_03H_Auxiliary_Input(Memory *memory, uint16_t unused);
//uint8_t function_04H_Auxiliary_Output(Memory *memory, uint16_t character);
uint8_t function_05H_Print_Character(Memory *memory, uint16_t character);
uint8_t function_06H_Direct_Console_IO(Memory *memory, uint16_t unused);
//uint8_t function_07H_Unfiltered_Character_Input_Without_Echo(uint8_t *character);
uint8_t function_08H_Character_Input_Without_Echo(Memory *memory, uint16_t character);
//
uint8_t function_0BH_Check_Keyboard_Status(Memory *memory, uint16_t unused);

// File system calls

uint8_t function_0FH_Open_File(Memory *memory, uint16_t fcb);
uint8_t function_10H_Close_File(Memory *memory, uint16_t fcb);
uint8_t function_11H_Find_First_File(Memory *memory, uint16_t fcb);
uint8_t function_12H_Find_Next_File(Memory *memory, uint16_t fcb);
uint8_t function_13H_Delete_File(Memory *memory, uint16_t fcb);
uint8_t function_14H_Sequential_Read(Memory *memory, uint16_t fcb);
uint8_t function_15H_Sequential_Write(Memory *memory, uint16_t fcb);
uint8_t function_16H_Create_File_With_FCB(Memory *memory, uint16_t fcb);
//uint8_t function_17H_Rename_File(uint8_t *fcb);
// 18H
uint8_t function_19H_Get_Current_Disk(Memory *memory, uint16_t unused);
uint8_t function_1AH_Set_DTA_Address(Memory *memory, uint16_t dta);

uint8_t function_21H_Random_Read(Memory *memory, uint16_t fcb);
uint8_t function_22H_Random_Write(Memory *memory, uint16_t fcb);
uint8_t function_23H_Get_File_Size(Memory *memory, uint16_t fcb);

typedef uint8_t (*SysCallFunction)(Memory*, uint16_t);

extern SysCallFunction DOS_SYSCALLS[256];

#endif
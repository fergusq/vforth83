#ifndef _ERRORS_H
#define _ERRORS_H

enum ErrorCode {
    ERROR_UNKNOWN_ERROR = 1,
    ERROR_DATA_STACK_UNDERFLOW,
    ERROR_DATA_STACK_OVERFLOW,
    ERROR_RETURN_STACK_UNDERFLOW,
    ERROR_RETURN_STACK_OVERFLOW,
    ERROR_RETURN_STACK_MISMATCH,
    ERROR_DEBUG_STACK_UNDERFLOW,
    ERROR_DEBUG_STACK_OVERFLOW,
    ERROR_MEMORY_ALLOCATION_ERROR,
    ERROR_DIVISION_BY_ZERO,
    ERROR_INVALID_MEMORY_ACCESS,
    ERROR_END_OF_INPUT,
    ERROR_WORD_NOT_FOUND,
    ERROR_WORD_NOT_EXECUTABLE,
    ERROR_FILE_NOT_FOUND,
    ERROR_FILE_NOT_OPEN,
    ERROR_FILE_TOO_LARGE,
    ERROR_UNKNOWN_SYSCALL,
    ERROR_UNKNOWN_DEFINITION_TYPE,
};

char *get_error_string(enum ErrorCode error);

#endif
#include "errors.h"

char *get_error_string(enum ErrorCode error) {
    switch (error)
    {
    case ERROR_UNKNOWN_ERROR: return "Unknown Error";
    case ERROR_DATA_STACK_UNDERFLOW: return "Stack Underflow";
    case ERROR_DATA_STACK_OVERFLOW: return "Stack Overflow";
    case ERROR_RETURN_STACK_UNDERFLOW: return "Return Stack Underflow";
    case ERROR_RETURN_STACK_OVERFLOW: return "Return Stack Overflow";
    case ERROR_RETURN_STACK_MISMATCH: return "Return Stack Mismatch";
    case ERROR_DEBUG_STACK_UNDERFLOW: return "Debug Stack Underflow";
    case ERROR_DEBUG_STACK_OVERFLOW: return "Debug Stack Overflow";
    case ERROR_MEMORY_ALLOCATION_ERROR: return "Memory Allocation Error";
    case ERROR_DIVISION_BY_ZERO: return "Division By Zero";
    case ERROR_INVALID_MEMORY_ACCESS: return "Invalid Memory Access";
    case ERROR_END_OF_INPUT: return "End Of Input";
    case ERROR_WORD_NOT_FOUND: return "Word Not Found";
    case ERROR_WORD_NOT_EXECUTABLE: return "Word Not Executable";
    case ERROR_FILE_NOT_FOUND: return "File Not Found";
    case ERROR_FILE_NOT_OPEN: return "File Not Open";
    case ERROR_FILE_TOO_LARGE: return "File Too Large";
    case ERROR_UNKNOWN_SYSCALL: return "Unknown BDOS Syscall";
    case ERROR_UNKNOWN_DEFINITION_TYPE: return "Unknown definition type";
    default: return "Unknown Error";
    }
}
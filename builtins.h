#ifndef _BUILTINS_H
#define _BUILTINS_H

enum BuiltinWord {
    BUILTIN_WORD_LIT = 0,

    // Required word set
    BUILTIN_WORD_STORE,
    BUILTIN_WORD_SHARP,
    BUILTIN_WORD_SHARP_GREATER,
    BUILTIN_WORD_SHARP_S,
    BUILTIN_WORD_NUMBER_TIB,
    BUILTIN_WORD_TICK,
    BUILTIN_WORD_PAREN,
    BUILTIN_WORD_TIMES,
    BUILTIN_WORD_TIMES_DIVIDE,
    BUILTIN_WORD_TIMES_DIVIDE_MOD,
    BUILTIN_WORD_PLUS,
    BUILTIN_WORD_PLUS_STORE,
    BUILTIN_WORD_PLUS_LOOP,
    BUILTIN_WORD_COMMA,
    BUILTIN_WORD_MINUS,
    BUILTIN_WORD_DASH_TRAILING,
    BUILTIN_WORD_DOT,
    BUILTIN_WORD_DOT_QUOTE,
    BUILTIN_WORD_DOT_PAREN,
    BUILTIN_WORD_DIVIDE,
    BUILTIN_WORD_DIVIDE_MOD,
    BUILTIN_WORD_ZERO_LESS,
    BUILTIN_WORD_ZERO_EQUALS,
    BUILTIN_WORD_ZERO_GREATER,
    BUILTIN_WORD_ONE_PLUS,
    BUILTIN_WORD_ONE_MINUS,
    BUILTIN_WORD_TWO_PLUS,
    BUILTIN_WORD_TWO_MINUS,
    BUILTIN_WORD_TWO_DIVIDE,
    BUILTIN_WORD_COLON,
    BUILTIN_WORD_SEMI_COLON,
    BUILTIN_WORD_LESS_THAN,
    BUILTIN_WORD_LESS_SHARP,
    BUILTIN_WORD_EQUALS,
    BUILTIN_WORD_GREATER_THAN,
    BUILTIN_WORD_TO_BODY,
    BUILTIN_WORD_TO_IN,
    BUILTIN_WORD_TO_R,
    BUILTIN_WORD_QUESTION_DUP,
    BUILTIN_WORD_FETCH,
    BUILTIN_WORD_ABORT,
    BUILTIN_WORD_ABORT_QUOTE,
    BUILTIN_WORD_ABS,
    BUILTIN_WORD_ALLOT,
    BUILTIN_WORD_AND,
    BUILTIN_WORD_BASE,
    BUILTIN_WORD_BEGIN,
    BUILTIN_WORD_BLK,
    BUILTIN_WORD_BLOCK,
    BUILTIN_WORD_BUFFER,
    BUILTIN_WORD_C_STORE,
    BUILTIN_WORD_C_FETCH,
    BUILTIN_WORD_CMOVE,
    BUILTIN_WORD_CMOVE_UP,
    BUILTIN_WORD_COMPILE,
    BUILTIN_WORD_CONSTANT,
    BUILTIN_WORD_CONVERT,
    BUILTIN_WORD_COUNT,
    BUILTIN_WORD_CR,
    BUILTIN_WORD_CREATE,
    BUILTIN_WORD_D_PLUS,
    BUILTIN_WORD_D_LESS_THAN,
    BUILTIN_WORD_DECIMAL,
    BUILTIN_WORD_DEFINITIONS,
    BUILTIN_WORD_DEPTH,
    BUILTIN_WORD_DNEGATE,
    BUILTIN_WORD_DO,
    BUILTIN_WORD_DOES,
    BUILTIN_WORD_DROP,
    BUILTIN_WORD_DUP,
    BUILTIN_WORD_ELSE,
    BUILTIN_WORD_EMIT,
    BUILTIN_WORD_EXECUTE,
    BUILTIN_WORD_EXIT,
    BUILTIN_WORD_EXPECT,
    BUILTIN_WORD_FILL,
    BUILTIN_WORD_FIND,
    BUILTIN_WORD_FLUSH,
    BUILTIN_WORD_FORGET,
    BUILTIN_WORD_FORTH,
    BUILTIN_WORD_FORTH_83,
    BUILTIN_WORD_HERE,
    BUILTIN_WORD_HOLD,
    BUILTIN_WORD_I,
    BUILTIN_WORD_IF,
    BUILTIN_WORD_IMMEDIATE,
    BUILTIN_WORD_J,
    BUILTIN_WORD_KEY,
    BUILTIN_WORD_LEAVE,
    BUILTIN_WORD_LITERAL,
    BUILTIN_WORD_LOAD,
    BUILTIN_WORD_LOOP,
    BUILTIN_WORD_MAX,
    BUILTIN_WORD_MIN,
    BUILTIN_WORD_MOD,
    BUILTIN_WORD_NEGATE,
    BUILTIN_WORD_NOT,
    BUILTIN_WORD_OR,
    BUILTIN_WORD_OVER,
    BUILTIN_WORD_PAD,
    BUILTIN_WORD_PICK,
    BUILTIN_WORD_QUIT,
    BUILTIN_WORD_R_FROM,
    BUILTIN_WORD_R_FETCH,
    BUILTIN_WORD_REPEAT,
    BUILTIN_WORD_ROLL,
    BUILTIN_WORD_ROT,
    BUILTIN_WORD_SAVE_BUFFERS,
    BUILTIN_WORD_SIGN,
    BUILTIN_WORD_SPACE,
    BUILTIN_WORD_SPACES,
    BUILTIN_WORD_SPAN,
    BUILTIN_WORD_STATE,
    BUILTIN_WORD_SWAP,
    BUILTIN_WORD_THEN,
    BUILTIN_WORD_TIB,
    BUILTIN_WORD_TYPE,
    BUILTIN_WORD_U_DOT,
    BUILTIN_WORD_U_LESS_THAN,
    BUILTIN_WORD_UM_TIMES,
    BUILTIN_WORD_UM_DIVIDE_MOD,
    BUILTIN_WORD_UPDATE,
    BUILTIN_WORD_VARIABLE,
    BUILTIN_WORD_VOCABULARY,
    BUILTIN_WORD_WHILE,
    BUILTIN_WORD_WORD,
    BUILTIN_WORD_XOR,
    BUILTIN_WORD_LEFT_BRACKET,
    BUILTIN_WORD_BRACKET_TICK,
    BUILTIN_WORD_BRACKET_COMPILE,
    BUILTIN_WORD_RIGHT_BRACKET,

    // Double number extension word set

    BUILTIN_WORD_TWO_STORE,
    BUILTIN_WORD_TWO_FETCH,
    BUILTIN_WORD_TWO_CONSTANT,
    BUILTIN_WORD_TWO_DROP,
    BUILTIN_WORD_TWO_DUP,
    BUILTIN_WORD_TWO_OVER,
    BUILTIN_WORD_TWO_ROT,
    BUILTIN_WORD_TWO_SWAP,
    BUILTIN_WORD_TWO_VARIABLE,
    BUILTIN_WORD_D_MINUS,
    BUILTIN_WORD_D_DOT,
    BUILTIN_WORD_D_DOT_R,
    BUILTIN_WORD_D_ZERO_EQUALS,
    BUILTIN_WORD_D_TWO_DIVIDE,
    BUILTIN_WORD_D_EQUAL,
    BUILTIN_WORD_DABS,
    BUILTIN_WORD_DMAX,
    BUILTIN_WORD_DMIN,
    BUILTIN_WORD_DU_LESS_THAN,

    // Assembler extension word set

    BUILTIN_WORD_SEMI_COLON_CODE,
    BUILTIN_WORD_ASSEMBLER,
    BUILTIN_WORD_CODE,
    BUILTIN_WORD_END_CODE,

    // System extension word set

    BUILTIN_WORD_BACKWARD_MARK,
    BUILTIN_WORD_BACKWARD_RESOLVE,
    BUILTIN_WORD_FORWARD_MARK,
    BUILTIN_WORD_FORWARD_RESOLVE,
    BUILTIN_WORD_QUESTION_BRANCH,
    BUILTIN_WORD_BRANCH,
    BUILTIN_WORD_CONTEXT,
    BUILTIN_WORD_CURRENT,

    // Controlled reference words

    BUILTIN_WORD_NEXT_BLOCK,
    BUILTIN_WORD_DOT_R,
    BUILTIN_WORD_TWO_TIMES,
    BUILTIN_WORD_BL,
    BUILTIN_WORD_BLANK,
    BUILTIN_WORD_C_COMMA,
    BUILTIN_WORD_DUMP,
    BUILTIN_WORD_EDITOR,
    BUILTIN_WORD_EMPTY_BUFFERS,
    BUILTIN_WORD_END,
    BUILTIN_WORD_ERASE,
    BUILTIN_WORD_HEX,
    BUILTIN_WORD_INTERPRET,
    BUILTIN_WORD_K,
    BUILTIN_WORD_LIST,
    BUILTIN_WORD_OCTAL,
    BUILTIN_WORD_OFFSET,
    BUILTIN_WORD_QUERY,
    BUILTIN_WORD_RECURSE,
    BUILTIN_WORD_SCR,
    BUILTIN_WORD_SP_FETCH,
    BUILTIN_WORD_THRU,
    BUILTIN_WORD_U_DOT_R,

    // Search order word set

    BUILTIN_WORD_ONLY,
    BUILTIN_WORD_ALSO,
    BUILTIN_WORD_ORDER,
    BUILTIN_WORD_WORDS,
    BUILTIN_WORD_SEAL,

    // Additional words

    BUILTIN_WORD_INCLUDE,
    BUILTIN_WORD_FROM,
    
    // Debug words

    BUILTIN_WORD_SEE,
};

#include "forth.h"

typedef void (*add_builtin_func)(char *name, enum BuiltinWord word, uint8_t is_immediate);

void add_builtins(add_builtin_func add_builtin);

int execute_builtin(InterpreterState *state, enum BuiltinWord word);

#endif
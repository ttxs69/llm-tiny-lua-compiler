#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdint.h>

typedef enum {
    OP_CONSTANT,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_SET_GLOBAL,
    OP_GET_GLOBAL,
    OP_POP,
    OP_PRINT,
    OP_JUMP_IF_FALSE,
    OP_JUMP,
    OP_GREATER,
    OP_LESS,
    OP_RETURN,
} OpCode;

typedef enum {
    VAL_NUMBER,
    VAL_STRING,
} ValueType;

typedef struct {
    ValueType type;
    union {
        double number;
        char* string;
    } as;
} Value;

typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    int* lines;
    // For constants
    Value* constants;
    int constants_count;
    int constants_capacity;
} Chunk;

void init_chunk(Chunk* chunk);
void write_chunk(Chunk* chunk, uint8_t byte, int line);
void write_short(Chunk* chunk, uint16_t value, int line);
int add_constant(Chunk* chunk, Value value);
void free_chunk(Chunk* chunk);

#endif // BYTECODE_H

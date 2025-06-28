#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdint.h>

typedef enum {
    OP_CONSTANT,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_PRINT,
    OP_RETURN,
} OpCode;

typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    // For constants
    double* constants;
    int constants_count;
    int constants_capacity;
} Chunk;

void init_chunk(Chunk* chunk);
void write_chunk(Chunk* chunk, uint8_t byte);
int add_constant(Chunk* chunk, double value);
void free_chunk(Chunk* chunk);

#endif // BYTECODE_H

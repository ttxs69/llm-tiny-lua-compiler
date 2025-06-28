#ifndef BYTECODE_H
#define BYTECODE_H

#include "chunk.h"
#include <stdio.h>

typedef enum {
    OP_CONSTANT,
    OP_SET_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_LOCAL,
    OP_GET_LOCAL,
    OP_POP,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE,
    OP_GREATER,
    OP_GREATER_EQUAL,
    OP_LESS,
    OP_LESS_EQUAL,
    OP_EQUAL,
    OP_NOT_EQUAL,
    OP_NOT,
    OP_CONCAT,
    OP_PRINT,
    OP_JUMP_IF_FALSE,
    OP_JUMP,
    OP_CALL,
    OP_RETURN,
    OP_TRUE,
    OP_FALSE,
    OP_NIL
} OpCode;

void init_chunk(Chunk* chunk);
void write_chunk(Chunk* chunk, uint8_t byte, int line);
void write_short(Chunk* chunk, uint16_t value, int line);
int add_constant(Chunk* chunk, Value value);
void free_chunk(Chunk* chunk);
int disassemble_instruction(Chunk* chunk, int offset);
void disassemble_instruction_to_stream(FILE* stream, Chunk* chunk, int offset);

#endif // BYTECODE_H

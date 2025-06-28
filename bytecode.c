#include "bytecode.h"
#include "value.h"
#include <stdlib.h>
#include <stdio.h>

static int simple_instruction(const char* name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

static int constant_instruction(const char* name, Chunk* chunk, int offset) {
    uint8_t constant_index = chunk->code[offset + 1];
    printf("%-16s %4d '", name, constant_index);
    print_value(chunk->constants[constant_index]);
    printf("'\n");
    return offset + 2;
}


static int short_instruction(const char* name, Chunk* chunk, int offset) {
    uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8 | chunk->code[offset + 2]);
    printf("%-16s %4d\n", name, offset + 3 + jump);
    return offset + 3;
}

int disassemble_instruction(Chunk* chunk, int offset) {
    printf("%04d ", offset);
    if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
        printf("   | ");
    } else {
        printf("%4d ", chunk->lines[offset]);
    }

    uint8_t instruction = chunk->code[offset];
    switch (instruction) {
        case OP_RETURN:
            return simple_instruction("OP_RETURN", offset);
        case OP_CONSTANT:
            return constant_instruction("OP_CONSTANT", chunk, offset);
        case OP_ADD:
            return simple_instruction("OP_ADD", offset);
        case OP_SUBTRACT:
            return simple_instruction("OP_SUBTRACT", offset);
        case OP_MULTIPLY:
            return simple_instruction("OP_MULTIPLY", offset);
        case OP_DIVIDE:
            return simple_instruction("OP_DIVIDE", offset);
        case OP_GREATER:
            return simple_instruction("OP_GREATER", offset);
        case OP_LESS:
            return simple_instruction("OP_LESS", offset);
        case OP_PRINT:
            return simple_instruction("OP_PRINT", offset);
        case OP_POP:
            return simple_instruction("OP_POP", offset);
        case OP_SET_GLOBAL:
            return constant_instruction("OP_SET_GLOBAL", chunk, offset);
        case OP_GET_GLOBAL:
            return constant_instruction("OP_GET_GLOBAL", chunk, offset);
        case OP_JUMP_IF_FALSE:
            return short_instruction("OP_JUMP_IF_FALSE", chunk, offset);
        case OP_JUMP:
            return short_instruction("OP_JUMP", chunk, offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}

/**
 * @brief Initializes a chunk.
 * 
 * @param chunk The chunk to initialize.
 */

void init_chunk(Chunk* chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    chunk->constants = NULL;
    chunk->constants_count = 0;
    chunk->constants_capacity = 0;
}

/**
 * @brief Writes a byte to a chunk.
 * 
 * @param chunk The chunk to write to.
 * @param byte The byte to write.
 * @param line The line number of the byte.
 */
void write_chunk(Chunk* chunk, uint8_t byte, int line) {
    if (chunk->capacity < chunk->count + 1) {
        int old_capacity = chunk->capacity;
        chunk->capacity = old_capacity < 8 ? 8 : old_capacity * 2;
        chunk->code = (uint8_t*)realloc(chunk->code, chunk->capacity);
        chunk->lines = (int*)realloc(chunk->lines, sizeof(int) * chunk->capacity);
    }
    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

/**
 * @brief Writes a 16-bit value to a chunk.
 * 
 * @param chunk The chunk to write to.
 * @param value The value to write.
 * @param line The line number of the value.
 */
void write_short(Chunk* chunk, uint16_t value, int line) {
    write_chunk(chunk, (value >> 8) & 0xFF, line);
    write_chunk(chunk, value & 0xFF, line);
}

/**
 * @brief Adds a constant to a chunk.
 * 
 * @param chunk The chunk to add the constant to.
 * @param value The constant to add.
 * @return The index of the added constant.
 */
int add_constant(Chunk* chunk, Value value) {
    if (chunk->constants_capacity < chunk->constants_count + 1) {
        int old_capacity = chunk->constants_capacity;
        chunk->constants_capacity = old_capacity < 8 ? 8 : old_capacity * 2;
        chunk->constants = (Value*)realloc(chunk->constants, sizeof(Value) * chunk->constants_capacity);
    }
    chunk->constants[chunk->constants_count] = value;
    return chunk->constants_count++;
}

/**
 * @brief Frees a chunk.
 * 
 * @param chunk The chunk to free.
 */
void free_chunk(Chunk* chunk) {
    free(chunk->code);
    free(chunk->lines);
    for (int i = 0; i < chunk->constants_count; i++) {
        if (chunk->constants[i].type == VAL_STRING) {
            free(chunk->constants[i].as.string);
        }
    }
    free(chunk->constants);
    init_chunk(chunk);
}

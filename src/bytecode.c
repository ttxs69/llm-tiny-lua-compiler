#include "bytecode.h"
#include "value.h"
#include <stdio.h>
#include <stdlib.h>

static int local_instruction(const char* name, Chunk* chunk, int offset, FILE* stream) {
    uint8_t local_index = chunk->code[offset + 1];
    fprintf(stream, "%-16s %4d '%s'\n", name, local_index, chunk->locals[local_index]);
    return offset + 2;
}

static int simple_instruction(const char* name, int offset, FILE* stream) {
    fprintf(stream, "%s\n", name);
    return offset + 1;
}

static int constant_instruction(const char* name, Chunk* chunk, int offset, FILE* stream) {
    uint8_t constant_index = chunk->code[offset + 1];
    fprintf(stream, "%-16s %4d '", name, constant_index);
    print_value_to_stream(stream, chunk->constants[constant_index]);
    fprintf(stream, "'\n");
    return offset + 2;
}


static int short_instruction(const char* name, Chunk* chunk, int offset, FILE* stream) {
    uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8 | chunk->code[offset + 2]);
    fprintf(stream, "%-16s %4d\n", name, offset + 3 + jump);
    return offset + 3;
}

void disassemble_instruction_to_stream(FILE* stream, Chunk* chunk, int offset) {
    fprintf(stream, "%04d ", offset);
    if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
        fprintf(stream, "   | ");
    } else {
        fprintf(stream, "%4d ", chunk->lines[offset]);
    }

    uint8_t instruction = chunk->code[offset];
    switch (instruction) {
        case OP_CALL:
            simple_instruction("OP_CALL", offset, stream);
            break;
        case OP_RETURN:
            simple_instruction("OP_RETURN", offset, stream);
            break;
        case OP_CONSTANT:
            constant_instruction("OP_CONSTANT", chunk, offset, stream);
            break;
        case OP_ADD:
            simple_instruction("OP_ADD", offset, stream);
            break;
        case OP_SUBTRACT:
            simple_instruction("OP_SUBTRACT", offset, stream);
            break;
        case OP_MULTIPLY:
            simple_instruction("OP_MULTIPLY", offset, stream);
            break;
        case OP_DIVIDE:
            simple_instruction("OP_DIVIDE", offset, stream);
            break;
        case OP_NEGATE:
            simple_instruction("OP_NEGATE", offset, stream);
            break;
        case OP_GREATER:
            simple_instruction("OP_GREATER", offset, stream);
            break;
        case OP_GREATER_EQUAL:
            simple_instruction("OP_GREATER_EQUAL", offset, stream);
            break;
        case OP_LESS:
            simple_instruction("OP_LESS", offset, stream);
            break;
        case OP_LESS_EQUAL:
            simple_instruction("OP_LESS_EQUAL", offset, stream);
            break;
        case OP_EQUAL:
            simple_instruction("OP_EQUAL", offset, stream);
            break;
        case OP_NOT_EQUAL:
            simple_instruction("OP_NOT_EQUAL", offset, stream);
            break;
        case OP_PRINT:
            simple_instruction("OP_PRINT", offset, stream);
            break;
        case OP_POP:
            simple_instruction("OP_POP", offset, stream);
            break;
        case OP_SET_GLOBAL:
            constant_instruction("OP_SET_GLOBAL", chunk, offset, stream);
            break;
        case OP_GET_GLOBAL:
            constant_instruction("OP_GET_GLOBAL", chunk, offset, stream);
            break;
        case OP_SET_LOCAL:
            local_instruction("OP_SET_LOCAL", chunk, offset, stream);
            break;
        case OP_GET_LOCAL:
            local_instruction("OP_GET_LOCAL", chunk, offset, stream);
            break;
        case OP_JUMP_IF_FALSE:
            short_instruction("OP_JUMP_IF_FALSE", chunk, offset, stream);
            break;
        case OP_JUMP:
            short_instruction("OP_JUMP", chunk, offset, stream);
            break;
        case OP_TRUE:
            simple_instruction("OP_TRUE", offset, stream);
            break;
        case OP_FALSE:
            simple_instruction("OP_FALSE", offset, stream);
            break;
        case OP_NIL:
            simple_instruction("OP_NIL", offset, stream);
            break;
        case OP_NOT:
            simple_instruction("OP_NOT", offset, stream);
            break;
        case OP_CONCAT:
            simple_instruction("OP_CONCAT", offset, stream);
            break;
        default:
            fprintf(stream, "Unknown opcode %d\n", instruction);
            break;
    }
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
        case OP_CALL:
            return simple_instruction("OP_CALL", offset, stdout);
        case OP_RETURN:
            return simple_instruction("OP_RETURN", offset, stdout);
        case OP_CONSTANT:
            return constant_instruction("OP_CONSTANT", chunk, offset, stdout);
        case OP_ADD:
            return simple_instruction("OP_ADD", offset, stdout);
        case OP_SUBTRACT:
            return simple_instruction("OP_SUBTRACT", offset, stdout);
        case OP_MULTIPLY:
            return simple_instruction("OP_MULTIPLY", offset, stdout);
        case OP_DIVIDE:
            return simple_instruction("OP_DIVIDE", offset, stdout);
        case OP_NEGATE:
            return simple_instruction("OP_NEGATE", offset, stdout);
        case OP_GREATER:
            return simple_instruction("OP_GREATER", offset, stdout);
        case OP_GREATER_EQUAL:
            return simple_instruction("OP_GREATER_EQUAL", offset, stdout);
        case OP_LESS:
            return simple_instruction("OP_LESS", offset, stdout);
        case OP_LESS_EQUAL:
            return simple_instruction("OP_LESS_EQUAL", offset, stdout);
        case OP_EQUAL:
            return simple_instruction("OP_EQUAL", offset, stdout);
        case OP_NOT_EQUAL:
            return simple_instruction("OP_NOT_EQUAL", offset, stdout);
        case OP_PRINT:
            return simple_instruction("OP_PRINT", offset, stdout);
        case OP_POP:
            return simple_instruction("OP_POP", offset, stdout);
        case OP_SET_GLOBAL:
            return constant_instruction("OP_SET_GLOBAL", chunk, offset, stdout);
        case OP_GET_GLOBAL:
            return constant_instruction("OP_GET_GLOBAL", chunk, offset, stdout);
        case OP_SET_LOCAL:
            return local_instruction("OP_SET_LOCAL", chunk, offset, stdout);
        case OP_GET_LOCAL:
            return local_instruction("OP_GET_LOCAL", chunk, offset, stdout);
        case OP_JUMP_IF_FALSE:
            return short_instruction("OP_JUMP_IF_FALSE", chunk, offset, stdout);
        case OP_JUMP:
            return short_instruction("OP_JUMP", chunk, offset, stdout);
        case OP_TRUE:
            return simple_instruction("OP_TRUE", offset, stdout);
        case OP_FALSE:
            return simple_instruction("OP_FALSE", offset, stdout);
        case OP_NIL:
            return simple_instruction("OP_NIL", offset, stdout);
        case OP_NOT:
            return simple_instruction("OP_NOT", offset, stdout);
        case OP_CONCAT:
            return simple_instruction("OP_CONCAT", offset, stdout);
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
    chunk->arity = 0;
    chunk->locals_count = 0;
    chunk->locals = NULL;
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
        free_value(chunk->constants[i]);
    }
    for (int i = 0; i < chunk->locals_count; i++) {
        free(chunk->locals[i]);
    }
    free(chunk->locals);
    free(chunk->constants);
    init_chunk(chunk);
}

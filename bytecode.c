#include "bytecode.h"
#include <stdlib.h>

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

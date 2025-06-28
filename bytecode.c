#include "bytecode.h"
#include <stdlib.h>

void init_chunk(Chunk* chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->constants = NULL;
    chunk->constants_count = 0;
    chunk->constants_capacity = 0;
}

void write_chunk(Chunk* chunk, uint8_t byte) {
    if (chunk->capacity < chunk->count + 1) {
        int old_capacity = chunk->capacity;
        chunk->capacity = old_capacity < 8 ? 8 : old_capacity * 2;
        chunk->code = (uint8_t*)realloc(chunk->code, chunk->capacity);
    }
    chunk->code[chunk->count] = byte;
    chunk->count++;
}

int add_constant(Chunk* chunk, double value) {
    if (chunk->constants_capacity < chunk->constants_count + 1) {
        int old_capacity = chunk->constants_capacity;
        chunk->constants_capacity = old_capacity < 8 ? 8 : old_capacity * 2;
        chunk->constants = (double*)realloc(chunk->constants, sizeof(double) * chunk->constants_capacity);
    }
    chunk->constants[chunk->constants_count] = value;
    return chunk->constants_count++;
}

void free_chunk(Chunk* chunk) {
    free(chunk->code);
    free(chunk->constants);
    init_chunk(chunk);
}

#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>
#include "value.h"

typedef struct Chunk {
    int count;
    int capacity;
    uint8_t* code;
    int* lines;
    // For constants
    Value* constants;
    int constants_count;
    int constants_capacity;
    int arity;
    int locals_count;
    char** locals;
} Chunk;

#endif // CHUNK_H

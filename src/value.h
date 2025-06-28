#ifndef VALUE_H
#define VALUE_H

#include <stdint.h>

typedef enum {
    VAL_NUMBER,
    VAL_STRING,
    VAL_TRUE,
    VAL_FALSE,
    VAL_NIL,
    VAL_FUNCTION,
} ValueType;

typedef struct Value {
    ValueType type;
    union {
        double number;
        char* string;
        int boolean;
        struct Chunk* function;
    } as;
} Value;

void print_value(Value value);
void free_value(Value value);

#endif // VALUE_H

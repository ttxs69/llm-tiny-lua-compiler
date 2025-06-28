#include "value.h"
#include "bytecode.h"
#include <stdio.h>
#include <stdlib.h>

void print_value(Value value) {
    switch (value.type) {
        case VAL_NUMBER:
            printf("%f", value.as.number);
            break;
        case VAL_STRING:
            printf("%s", value.as.string);
            break;
        case VAL_TRUE:
            printf("true");
            break;
        case VAL_FALSE:
            printf("false");
            break;
        case VAL_NIL:
            printf("nil");
            break;
        case VAL_FUNCTION:
            printf("<function>");
            break;
    }
}

void free_value(Value value) {
    if (value.type == VAL_FUNCTION) {
        free_chunk(value.as.function);
        free(value.as.function);
    } else if (value.type == VAL_STRING) {
        free(value.as.string);
    }
}

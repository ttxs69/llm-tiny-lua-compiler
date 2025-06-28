#include "value.h"
#include "bytecode.h"
#include <stdio.h>
#include <stdlib.h>

void print_value_to_stream(FILE* stream, Value value) {
    switch (value.type) {
        case VAL_NUMBER:
            fprintf(stream, "%f", value.as.number);
            break;
        case VAL_STRING:
            fprintf(stream, "%s", value.as.string);
            break;
        case VAL_TRUE:
            fprintf(stream, "true");
            break;
        case VAL_FALSE:
            fprintf(stream, "false");
            break;
        case VAL_NIL:
            fprintf(stream, "nil");
            break;
        case VAL_FUNCTION:
            fprintf(stream, "<function>");
            break;
    }
}

void print_value(Value value) {
    print_value_to_stream(stdout, value);
}

void free_value(Value value) {
    if (value.type == VAL_FUNCTION) {
        free_chunk(value.as.function);
        free(value.as.function);
    } else if (value.type == VAL_STRING) {
        free(value.as.string);
    }
}

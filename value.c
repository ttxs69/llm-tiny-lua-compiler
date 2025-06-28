#include "value.h"
#include <stdio.h>

void print_value(Value value) {
    switch (value.type) {
        case VAL_NUMBER:
            printf("%g", value.as.number);
            break;
        case VAL_STRING:
            printf("%s", value.as.string);
            break;
    }
}

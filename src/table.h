#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>

#include "bytecode.h"

typedef struct {
    char* key;
    Value value;
} Entry;

typedef struct {
    int count;
    int capacity;
    Entry* entries;
} Table;

void init_table(Table* table);
void free_table(Table* table);
int table_set(Table* table, char* key, Value value);
int table_get(Table* table, char* key, Value* value);

#endif // TABLE_H

#include "table.h"
#include <stdlib.h>
#include <string.h>

void init_table(Table* table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void free_table(Table* table) {
    free(table->entries);
    init_table(table);
}

static uint32_t hash_string(const char* key, int length) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

static Entry* find_entry(Entry* entries, int capacity, char* key) {
    uint32_t hash = hash_string(key, strlen(key));
    uint32_t index = hash % capacity;

    for (;;) {
        Entry* entry = &entries[index];
        if (entry->key == NULL || strcmp(entry->key, key) == 0) {
            return entry;
        }
        index = (index + 1) % capacity;
    }
}

static void adjust_capacity(Table* table, int capacity) {
    Entry* entries = (Entry*)malloc(sizeof(Entry) * capacity);
    for (int i = 0; i < capacity; i++) {
        entries[i].key = NULL;
        entries[i].value.type = VAL_NUMBER;
        entries[i].value.as.number = 0;
    }

    for (int i = 0; i < table->count; i++) {
        Entry* dest = find_entry(entries, capacity, table->entries[i].key);
        dest->key = table->entries[i].key;
        dest->value = table->entries[i].value;
    }

    free(table->entries);
    table->entries = entries;
    table->capacity = capacity;
}

int table_set(Table* table, char* key, Value value) {
    if (table->count + 1 > table->capacity * 0.75) {
        int capacity = table->capacity < 8 ? 8 : table->capacity * 2;
        adjust_capacity(table, capacity);
    }

    Entry* entry = find_entry(table->entries, table->capacity, key);
    int is_new_key = entry->key == NULL;
    if (is_new_key) {
        table->count++;
    }

    entry->key = key;
    entry->value = value;
    return is_new_key;
}

int table_get(Table* table, char* key, Value* value) {
    if (table->count == 0) return 0;

    Entry* entry = find_entry(table->entries, table->capacity, key);
    if (entry->key == NULL) return 0;

    *value = entry->value;
    return 1;
}

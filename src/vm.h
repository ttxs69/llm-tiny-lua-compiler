#ifndef VM_H
#define VM_H

#include "bytecode.h"
#include "table.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * 256)

typedef struct {
    Chunk* chunk;
    uint8_t* ip;
    Value* slots;
} CallFrame;

typedef struct {
    CallFrame frames[FRAMES_MAX];
    int frame_count;

    Value stack[STACK_MAX];
    Value* stack_top;
    Table globals;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

void init_vm(VM* vm);
void free_vm(VM* vm);
InterpretResult interpret(VM* vm, const char* source);

#endif // VM_H

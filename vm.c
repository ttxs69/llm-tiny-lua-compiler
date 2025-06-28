#include "vm.h"
#include "parser.h"
#include "codegen.h"
#include <stdio.h>

void init_vm(VM* vm) {
    vm->chunk = NULL;
    vm->ip = NULL;
    vm->stack_top = vm->stack;
}

void free_vm(VM* vm) {
    // No need to free chunk here, it's owned by the compiler
}

static void push(VM* vm, double value) {
    *vm->stack_top = value;
    vm->stack_top++;
}

static double pop(VM* vm) {
    vm->stack_top--;
    return *vm->stack_top;
}

static InterpretResult run(VM* vm) {
#define READ_BYTE() (*vm->ip++)
#define READ_CONSTANT() (vm->chunk->constants[READ_BYTE()])

    for (;;) {
        uint8_t instruction = READ_BYTE();
        switch (instruction) {
            case OP_CONSTANT: {
                double constant = READ_CONSTANT();
                push(vm, constant);
                break;
            }
            case OP_ADD: {
                double b = pop(vm);
                double a = pop(vm);
                push(vm, a + b);
                break;
            }
            case OP_SUBTRACT: {
                double b = pop(vm);
                double a = pop(vm);
                push(vm, a - b);
                break;
            }
            case OP_MULTIPLY: {
                double b = pop(vm);
                double a = pop(vm);
                push(vm, a * b);
                break;
            }
            case OP_DIVIDE: {
                double b = pop(vm);
                double a = pop(vm);
                push(vm, a / b);
                break;
            }
            case OP_PRINT: {
                printf("%f\n", pop(vm));
                break;
            }
            case OP_RETURN: {
                return INTERPRET_OK;
            }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
}

InterpretResult interpret(VM* vm, const char* source) {
    Chunk chunk;
    init_chunk(&chunk);

    ASTNode* ast = parse(source);
    if (ast == NULL) {
        return INTERPRET_COMPILE_ERROR;
    }

    generate_code(ast, &chunk);
    vm->chunk = &chunk;
    vm->ip = vm->chunk->code;

    InterpretResult result = run(vm);

    free_chunk(&chunk);
    return result;
}
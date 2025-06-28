#include "vm.h"
#include "parser.h"
#include "codegen.h"
#include "bytecode.h"
#include "value.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/**
 * @brief Prints a runtime error message.
 * 
 * @param vm The VM.
 * @param format The format string.
 * @param ... The arguments.
 */
static void runtime_error(VM* vm, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm->ip - vm->chunk->code - 1;
    fprintf(stderr, "[line %d] in script\n", vm->chunk->lines[instruction]);
    vm->stack_top = vm->stack;
}

void init_vm(VM* vm) {
    vm->chunk = NULL;
    vm->ip = NULL;
    vm->stack_top = vm->stack;
    init_table(&vm->globals);
}

void free_vm(VM* vm) {
    free_table(&vm->globals);
}

/**
 * @brief Pushes a value onto the VM's stack.
 * 
 * @param vm The VM.
 * @param value The value to push.
 */
static void push(VM* vm, Value value) {
    *vm->stack_top = value;
    vm->stack_top++;
}

/**
 * @brief Pops a value from the VM's stack.
 * 
 * @param vm The VM.
 * @return The popped value.
 */
static Value pop(VM* vm) {
    vm->stack_top--;
    return *vm->stack_top;
}

/**
 * @brief The main execution loop of the VM.
 * 
 * @param vm The VM.
 * @return The result of the interpretation.
 */
static InterpretResult run(VM* vm) {
// Helper macros for reading from the bytecode
#define READ_BYTE() (*vm->ip++)
#define READ_SHORT() (vm->ip += 2, (uint16_t)((vm->ip[-2] << 8) | vm->ip[-1]))
#define READ_CONSTANT() (vm->chunk->constants[READ_BYTE()])
#define READ_STRING() (READ_CONSTANT().as.string)

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (Value* slot = vm->stack; slot < vm->stack_top; slot++) {
            printf("[ ");
            print_value(*slot);
            printf(" ]");
        }
        printf("\n");
        disassemble_instruction(vm->chunk, (int)(vm->ip - vm->chunk->code));
#endif
        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(vm, constant);
                break;
            }
            case OP_SET_GLOBAL: {
                char* name = READ_STRING();
                table_set(&vm->globals, name, *(vm->stack_top - 1));
                break;
            }
            case OP_GET_GLOBAL: {
                char* name = READ_STRING();
                Value value;
                if (!table_get(&vm->globals, name, &value)) {
                    runtime_error(vm, "Undefined variable '%s'.", name);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(vm, value);
                break;
            }
            case OP_POP: {
                pop(vm);
                break;
            }
            case OP_ADD: {
                Value b = pop(vm);
                Value a = pop(vm);
                if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
                    push(vm, (Value){VAL_NUMBER, {.number = a.as.number + b.as.number}});
                } else {
                    runtime_error(vm, "Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SUBTRACT: {
                Value b = pop(vm);
                Value a = pop(vm);
                if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
                    push(vm, (Value){VAL_NUMBER, {.number = a.as.number - b.as.number}});
                } else {
                    runtime_error(vm, "Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_MULTIPLY: {
                Value b = pop(vm);
                Value a = pop(vm);
                if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
                    push(vm, (Value){VAL_NUMBER, {.number = a.as.number * b.as.number}});
                } else {
                    runtime_error(vm, "Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_DIVIDE: {
                Value b = pop(vm);
                Value a = pop(vm);
                if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
                    push(vm, (Value){VAL_NUMBER, {.number = a.as.number / b.as.number}});
                } else {
                    runtime_error(vm, "Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_GREATER: {
                Value b = pop(vm);
                Value a = pop(vm);
                if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
                    push(vm, (Value){VAL_NUMBER, {.number = a.as.number > b.as.number}});
                } else {
                    runtime_error(vm, "Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_GREATER_EQUAL: {
                Value b = pop(vm);
                Value a = pop(vm);
                if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
                    push(vm, (Value){VAL_NUMBER, {.number = a.as.number >= b.as.number}});
                } else {
                    runtime_error(vm, "Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_LESS: {
                Value b = pop(vm);
                Value a = pop(vm);
                if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
                    push(vm, (Value){VAL_NUMBER, {.number = a.as.number < b.as.number}});
                } else {
                    runtime_error(vm, "Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_LESS_EQUAL: {
                Value b = pop(vm);
                Value a = pop(vm);
                if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
                    push(vm, (Value){VAL_NUMBER, {.number = a.as.number <= b.as.number}});
                } else {
                    runtime_error(vm, "Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_EQUAL: {
                Value b = pop(vm);
                Value a = pop(vm);
                if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
                    push(vm, (Value){VAL_NUMBER, {.number = a.as.number == b.as.number}});
                } else {
                    runtime_error(vm, "Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_NOT_EQUAL: {
                Value b = pop(vm);
                Value a = pop(vm);
                if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
                    push(vm, (Value){VAL_NUMBER, {.number = a.as.number != b.as.number}});
                } else {
                    runtime_error(vm, "Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_PRINT: {
                Value value = pop(vm);
                print_value(value);
                printf("\n");
                break;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if (pop(vm).as.number == 0) {
                    vm->ip += offset;
                }
                break;
            }
            case OP_JUMP: {
                int16_t offset = READ_SHORT();
                vm->ip += offset;
                break;
            }
            case OP_RETURN: {
                return INTERPRET_OK;
            }
            case OP_TRUE: {
                push(vm, (Value){VAL_TRUE, {.boolean = 1}});
                break;
            }
            case OP_FALSE: {
                push(vm, (Value){VAL_FALSE, {.boolean = 0}});
                break;
            }
            case OP_NIL: {
                push(vm, (Value){VAL_NIL});
                break;
            }
        }
    }

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
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
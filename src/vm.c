#include "vm.h"
#include "parser.h"
#include "codegen.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>

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

    CallFrame* frame = &vm->frames[vm->frame_count - 1];
    size_t instruction = frame->ip - frame->chunk->code - 1;
    fprintf(stderr, "[line %d] in script\n", frame->chunk->lines[instruction]);
    vm->stack_top = vm->stack;
}

void init_vm(VM* vm) {
    vm->frame_count = 0;
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

static int is_falsey(Value value) {
    return value.type == VAL_NIL || (value.type == VAL_FALSE && value.as.boolean == false);
}

static int call_value(VM* vm, Value callee, int arg_count) {
    if (callee.type != VAL_FUNCTION) {
        runtime_error(vm, "Can only call functions.");
        return 0;
    }

    struct Chunk* function = callee.as.function;
    if (arg_count != function->arity) {
        runtime_error(vm, "Expected %d arguments but got %d.", function->arity, arg_count);
        return 0;
    }

    if (vm->frame_count == FRAMES_MAX) {
        runtime_error(vm, "Stack overflow.");
        return 0;
    }

    CallFrame* frame = &vm->frames[vm->frame_count++];
    frame->chunk = function;
    frame->ip = function->code;
    frame->slots = vm->stack_top - arg_count;
    return 1;
}

/**
 * @brief The main execution loop of the VM.
 * 
 * @param vm The VM.
 * @return The result of the interpretation.
 */
static InterpretResult run(VM* vm) {
    CallFrame* frame = &vm->frames[vm->frame_count - 1];

// Helper macros for reading from the bytecode
#define READ_BYTE() (*frame->ip++)
#define READ_SHORT() (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define READ_CONSTANT() (frame->chunk->constants[READ_BYTE()])
#define READ_STRING() (READ_CONSTANT().as.string)

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        fprintf(stderr, "          ");
        for (Value* slot = vm->stack; slot < vm->stack_top; slot++) {
            fprintf(stderr, "[ ");
            print_value_to_stream(stderr, *slot);
            fprintf(stderr, " ]");
        }
        fprintf(stderr, "\n");
        disassemble_instruction_to_stream(stderr, frame->chunk, (int)(frame->ip - frame->chunk->code));
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
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(vm, frame->slots[slot]);
                break;
            }
            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = *(vm->stack_top - 1);
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
            case OP_NEGATE: {
                Value value = pop(vm);
                if (value.type == VAL_NUMBER) {
                    push(vm, (Value){VAL_NUMBER, {.number = -value.as.number}});
                } else {
                    runtime_error(vm, "Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_GREATER: {
                Value b = pop(vm);
                Value a = pop(vm);
                if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
                    push(vm, a.as.number > b.as.number ? (Value){VAL_TRUE, {.boolean = 1}} : (Value){VAL_FALSE, {.boolean = 0}});
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
                    push(vm, a.as.number >= b.as.number ? (Value){VAL_TRUE, {.boolean = 1}} : (Value){VAL_FALSE, {.boolean = 0}});
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
                    push(vm, a.as.number < b.as.number ? (Value){VAL_TRUE, {.boolean = 1}} : (Value){VAL_FALSE, {.boolean = 0}});
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
                    push(vm, a.as.number <= b.as.number ? (Value){VAL_TRUE, {.boolean = 1}} : (Value){VAL_FALSE, {.boolean = 0}});
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
                    push(vm, a.as.number == b.as.number ? (Value){VAL_TRUE, {.boolean = 1}} : (Value){VAL_FALSE, {.boolean = 0}});
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
                    push(vm, a.as.number != b.as.number ? (Value){VAL_TRUE, {.boolean = 1}} : (Value){VAL_FALSE, {.boolean = 0}});
                } else {
                    runtime_error(vm, "Operands must be numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_NOT:
                push(vm, is_falsey(pop(vm)) ? (Value){VAL_TRUE, {.boolean = 1}} : (Value){VAL_FALSE, {.boolean = 0}});
                break;
            case OP_CONCAT: {
                Value b = pop(vm);
                Value a = pop(vm);
                if (a.type == VAL_STRING && b.type == VAL_STRING) {
                    int length = strlen(a.as.string) + strlen(b.as.string);
                    char* result = (char*)malloc(length + 1);
                    memcpy(result, a.as.string, strlen(a.as.string));
                    memcpy(result + strlen(a.as.string), b.as.string, strlen(b.as.string));
                    result[length] = '\0';
                    push(vm, (Value){VAL_STRING, {.string = result}});
                } else {
                    runtime_error(vm, "Operands must be strings.");
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
                if (is_falsey(*(vm->stack_top - 1))) {
                    frame->ip += offset;
                }
                break;
            }
            case OP_JUMP: {
                int16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }
            case OP_CALL: {
                int arg_count = READ_BYTE();
                if (!call_value(vm, *(vm->stack_top - 1 - arg_count), arg_count)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm->frames[vm->frame_count - 1];
                break;
            }
            case OP_RETURN: {
                Value result = pop(vm);
                vm->frame_count--;
                if (vm->frame_count == 0) {
                    pop(vm);
                    return INTERPRET_OK;
                }
                vm->stack_top = frame->slots;
                push(vm, result);
                frame = &vm->frames[vm->frame_count - 1];
                break;
            }
            case OP_TRUE: {
                push(vm, (Value){VAL_TRUE, {.boolean = true}});
                break;
            }
            case OP_FALSE: {
                push(vm, (Value){VAL_FALSE, {.boolean = false}});
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

    struct ASTNode* ast = parse(source);
    if (ast == NULL) {
        return INTERPRET_COMPILE_ERROR;
    }

    generate_code(ast, &chunk);
    
    CallFrame* frame = &vm->frames[vm->frame_count++];
    frame->chunk = &chunk;
    frame->ip = chunk.code;
    frame->slots = vm->stack;

    InterpretResult result = run(vm);

    free_chunk(&chunk);
    return result;
}
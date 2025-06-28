#include "codegen.h"
#include "debug.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Forward declarations
static void generate_expression(struct ASTNode* node, Chunk* chunk);
static void generate_statement(struct ASTNode* node, Chunk* chunk);

/**
 * @brief Generates code for an expression.
 * 
 * @param node The expression node.
 * @param chunk The chunk to write the code to.
 */
static void generate_expression(struct ASTNode* node, Chunk* chunk) {
#ifdef DEBUG_TRACE_CODEGEN
    debug_log("Generating expression for node type %s\n", node_type_to_string(node->type));
#endif
    switch (node->type) {
        case NODE_NUMBER: {
            Value value = {VAL_NUMBER, {.number = node->data.number_value}};
            int constant_index = add_constant(chunk, value);
            write_chunk(chunk, OP_CONSTANT, node->line);
            write_chunk(chunk, constant_index, node->line);
            break;
        }
        case NODE_STRING: {
            Value value = {VAL_STRING, {.string = strdup(node->data.string_value)}};
            int constant_index = add_constant(chunk, value);
            write_chunk(chunk, OP_CONSTANT, node->line);
            write_chunk(chunk, constant_index, node->line);
            break;
        }
        case NODE_IDENTIFIER: {
            // Check if the identifier is a local variable
            int local_index = -1;
            for (int i = 0; i < chunk->locals_count; i++) {
                if (strcmp(chunk->locals[i], node->data.identifier_name) == 0) {
                    local_index = i;
                    break;
                }
            }

            if (local_index != -1) {
                write_chunk(chunk, OP_GET_LOCAL, node->line);
                write_chunk(chunk, local_index, node->line);
            } else {
                Value value = {VAL_STRING, {.string = strdup(node->data.identifier_name)}};
                int constant_index = add_constant(chunk, value);
                write_chunk(chunk, OP_GET_GLOBAL, node->line);
                write_chunk(chunk, constant_index, node->line);
            }
            break;
        }
        case NODE_BINARY_OP: {
            generate_expression(node->data.binary_op.left, chunk);
            generate_expression(node->data.binary_op.right, chunk);
            switch (node->data.binary_op.op) {
                case TOKEN_PLUS:          write_chunk(chunk, OP_ADD, node->line); break;
                case TOKEN_MINUS:         write_chunk(chunk, OP_SUBTRACT, node->line); break;
                case TOKEN_MUL:           write_chunk(chunk, OP_MULTIPLY, node->line); break;
                case TOKEN_DIV:           write_chunk(chunk, OP_DIVIDE, node->line); break;
                case TOKEN_GREATER:       write_chunk(chunk, OP_GREATER, node->line); break;
                case TOKEN_GREATER_EQUAL: write_chunk(chunk, OP_GREATER_EQUAL, node->line); break;
                case TOKEN_LESS:          write_chunk(chunk, OP_LESS, node->line); break;
                case TOKEN_LESS_EQUAL:    write_chunk(chunk, OP_LESS_EQUAL, node->line); break;
                case TOKEN_EQUAL:         write_chunk(chunk, OP_EQUAL, node->line); break;
                case TOKEN_NOT_EQUAL:     write_chunk(chunk, OP_NOT_EQUAL, node->line); break;
                case TOKEN_CONCAT:        write_chunk(chunk, OP_CONCAT, node->line); break;
                default: break; // Should not happen
            }
            break;
        }
        case NODE_UNARY_OP: {
            generate_expression(node->data.unary_op.right, chunk);
            switch (node->data.unary_op.op) {
                case TOKEN_MINUS: write_chunk(chunk, OP_NEGATE, node->line); break;
                case TOKEN_NOT: write_chunk(chunk, OP_NOT, node->line); break;
                default: break; // Should not happen
            }
            break;
        }
        case NODE_LOGICAL_OP: {
            generate_expression(node->data.logical_op.left, chunk);
            switch (node->data.logical_op.op) {
                case TOKEN_AND: {
                    write_chunk(chunk, OP_JUMP_IF_FALSE, node->line);
                    int end_jump = chunk->count;
                    write_short(chunk, 0, node->line);
                    write_chunk(chunk, OP_POP, node->line);
                    generate_expression(node->data.logical_op.right, chunk);
                    chunk->code[end_jump] = (chunk->count - end_jump - 2) >> 8;
                    chunk->code[end_jump + 1] = (chunk->count - end_jump - 2) & 0xFF;
                    break;
                }
                case TOKEN_OR: {
                    write_chunk(chunk, OP_JUMP_IF_FALSE, node->line);
                    int else_jump = chunk->count;
                    write_short(chunk, 0, node->line);
                    write_chunk(chunk, OP_JUMP, node->line);
                    int end_jump = chunk->count;
                    write_short(chunk, 0, node->line);
                    chunk->code[else_jump] = (chunk->count - else_jump - 2) >> 8;
                    chunk->code[else_jump + 1] = (chunk->count - else_jump - 2) & 0xFF;
                    write_chunk(chunk, OP_POP, node->line);
                    generate_expression(node->data.logical_op.right, chunk);
                    chunk->code[end_jump] = (chunk->count - end_jump - 2) >> 8;
                    chunk->code[end_jump + 1] = (chunk->count - end_jump - 2) & 0xFF;
                    break;
                }
                default: break; // Should not happen
            }
            break;
        }
        case NODE_FUNCTION_CALL: {
            // Get the function on the stack
            Value value = {VAL_STRING, {.string = strdup(node->data.function_call.function_name)}};
            int constant_index = add_constant(chunk, value);
            write_chunk(chunk, OP_GET_GLOBAL, node->line);
            write_chunk(chunk, constant_index, node->line);

            struct ASTNode* arg = node->data.function_call.argument;
            int arg_count = 0;
            while (arg) {
                generate_expression(arg, chunk);
                arg = arg->next;
                arg_count++;
            }
            write_chunk(chunk, OP_CALL, node->line);
            write_chunk(chunk, arg_count, node->line);
            break;
        }
        case NODE_TRUE:
            write_chunk(chunk, OP_TRUE, node->line);
            break;
        case NODE_FALSE:
            write_chunk(chunk, OP_FALSE, node->line);
            break;
        case NODE_NIL:
            write_chunk(chunk, OP_NIL, node->line);
            break;
        default:
            break; // Should not happen
    }
}

/**
 * @brief Generates code for a statement.
 * 
 * @param node The statement node.
 * @param chunk The chunk to write the code to.
 */
static void generate_statement(struct ASTNode* node, Chunk* chunk) {
#ifdef DEBUG_TRACE_CODEGEN
    debug_log("Generating statement for node type %s\n", node_type_to_string(node->type));
#endif
    switch (node->type) {

        case NODE_PRINT:
            generate_expression(node->data.print_statement.expression, chunk);
            write_chunk(chunk, OP_PRINT, node->line);
            break;
        case NODE_ASSIGN: {
            generate_expression(node->data.assignment.expression, chunk);
            Value value = {VAL_STRING, {.string = strdup(node->data.assignment.identifier)}};
            int constant_index = add_constant(chunk, value);
            write_chunk(chunk, OP_SET_GLOBAL, node->line);
            write_chunk(chunk, constant_index, node->line);
            break;
        }
        case NODE_IF: {
            generate_expression(node->data.if_statement.condition, chunk);

            // Emit jump instruction
            write_chunk(chunk, OP_JUMP_IF_FALSE, node->line);
            int else_jump = chunk->count;
            write_short(chunk, 0, node->line); // Placeholder for jump offset

            generate_statement(node->data.if_statement.then_branch, chunk);

            // Emit jump instruction to skip else block
            write_chunk(chunk, OP_JUMP, node->line);
            int exit_jump = chunk->count;
            write_short(chunk, 0, node->line); // Placeholder for jump offset

            // Patch else jump
            chunk->code[else_jump] = (chunk->count - else_jump - 2) >> 8;
            chunk->code[else_jump + 1] = (chunk->count - else_jump - 2) & 0xFF;

            if (node->data.if_statement.else_branch) {
                generate_statement(node->data.if_statement.else_branch, chunk);
            }

            // Patch exit jump
            chunk->code[exit_jump] = (chunk->count - exit_jump - 2) >> 8;
            chunk->code[exit_jump + 1] = (chunk->count - exit_jump - 2) & 0xFF;
            break;
        }
        case NODE_WHILE: {
            int loop_start = chunk->count;
            generate_expression(node->data.while_statement.condition, chunk);

            // Emit jump instruction
            write_chunk(chunk, OP_JUMP_IF_FALSE, node->line);
            int exit_jump = chunk->count;
            write_short(chunk, 0, node->line); // Placeholder for jump offset

            generate_statement(node->data.while_statement.body, chunk);

            // Emit jump to loop start
            write_chunk(chunk, OP_JUMP, node->line);
            write_short(chunk, (int16_t)(loop_start - chunk->count - 2), node->line);

            // Patch exit jump
            chunk->code[exit_jump] = (chunk->count - exit_jump - 2) >> 8;
            chunk->code[exit_jump + 1] = (chunk->count - exit_jump - 2) & 0xFF;
            break;
        }
        case NODE_STATEMENTS: {
            struct ASTNode* current = node->data.statements.statement;
            while (current) {
                generate_statement(current, chunk);
                current = current->next;
            }
            break;
        }
        case NODE_EXPRESSION_STATEMENT: {
            generate_expression(node->data.expression_statement.expression, chunk);
            write_chunk(chunk, OP_POP, node->line);
            break;
        }
        case NODE_FUNCTION_DEF: {
            Chunk* func_chunk = (Chunk*)malloc(sizeof(Chunk));
            init_chunk(func_chunk);
            func_chunk->locals_count = 0;
            
            struct ASTNode* param = node->data.function_def.parameters;
            while (param) {
                func_chunk->arity++;
                func_chunk->locals_count++;
                func_chunk->locals = (char**)realloc(func_chunk->locals, sizeof(char*) * func_chunk->locals_count);
                func_chunk->locals[func_chunk->locals_count - 1] = strdup(param->data.identifier_name);
                param = param->next;
            }

            generate_statement(node->data.function_def.body, func_chunk);
            write_chunk(func_chunk, OP_RETURN, node->line);

            Value func_val = {VAL_FUNCTION, {.function = func_chunk}};
            int constant_index = add_constant(chunk, func_val);
            write_chunk(chunk, OP_CONSTANT, node->line);
            write_chunk(chunk, constant_index, node->line);

            Value name_val = {VAL_STRING, {.string = strdup(node->data.function_def.function_name)}};
            constant_index = add_constant(chunk, name_val);
            write_chunk(chunk, OP_SET_GLOBAL, node->line);
            write_chunk(chunk, constant_index, node->line);
            break;
        }
        case NODE_RETURN: {
            generate_expression(node->data.return_statement.expression, chunk);
            write_chunk(chunk, OP_RETURN, node->line);
            break;
        }
        case NODE_LOCAL_DECLARATION: {
            if (node->data.local_declaration.expression) {
                generate_expression(node->data.local_declaration.expression, chunk);
            } else {
                write_chunk(chunk, OP_NIL, node->line);
            }
            chunk->locals = (char**)realloc(chunk->locals, sizeof(char*) * (chunk->locals_count + 1));
            chunk->locals[chunk->locals_count] = strdup(node->data.local_declaration.identifier);
            write_chunk(chunk, OP_SET_LOCAL, node->line);
            write_chunk(chunk, chunk->locals_count++, node->line);
            break;
        }
        default:
            break; // Should not happen
    }
}

/**
 * @brief Generates code for the given AST.
 * 
 * @param node The root of the AST.
 * @param chunk The chunk to write the code to.
 */
void generate_code(struct ASTNode* node, Chunk* chunk) {
    generate_statement(node, chunk);
    write_chunk(chunk, OP_RETURN, -1); // No line number for return
}
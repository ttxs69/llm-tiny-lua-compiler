#include "codegen.h"
#include <string.h>
#include <stdio.h>

// Forward declarations
static void generate_expression(ASTNode* node, Chunk* chunk);
static void generate_statement(ASTNode* node, Chunk* chunk);

/**
 * @brief Generates code for an expression.
 * 
 * @param node The expression node.
 * @param chunk The chunk to write the code to.
 */
static void generate_expression(ASTNode* node, Chunk* chunk) {
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
            Value value = {VAL_STRING, {.string = strdup(node->data.identifier_name)}};
            int constant_index = add_constant(chunk, value);
            write_chunk(chunk, OP_GET_GLOBAL, node->line);
            write_chunk(chunk, constant_index, node->line);
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
                default: break; // Should not happen
            }
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
static void generate_statement(ASTNode* node, Chunk* chunk) {
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
            write_chunk(chunk, OP_POP, node->line);
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
            ASTNode* current = node->data.statements.statement;
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
void generate_code(ASTNode* node, Chunk* chunk) {
    generate_statement(node, chunk);
    write_chunk(chunk, OP_RETURN, -1); // No line number for return
}
#include "codegen.h"

static void generate_expression(ASTNode* node, Chunk* chunk) {
    switch (node->type) {
        case NODE_NUMBER: {
            int constant_index = add_constant(chunk, node->data.number_value);
            write_chunk(chunk, OP_CONSTANT);
            write_chunk(chunk, constant_index);
            break;
        }
        case NODE_BINARY_OP: {
            generate_expression(node->data.binary_op.left, chunk);
            generate_expression(node->data.binary_op.right, chunk);
            switch (node->data.binary_op.op) {
                case TOKEN_PLUS:    write_chunk(chunk, OP_ADD); break;
                case TOKEN_MINUS:   write_chunk(chunk, OP_SUBTRACT); break;
                case TOKEN_MUL:     write_chunk(chunk, OP_MULTIPLY); break;
                case TOKEN_DIV:     write_chunk(chunk, OP_DIVIDE); break;
                default: break; // Should not happen
            }
            break;
        }
        default:
            break; // Should not happen
    }
}

static void generate_statement(ASTNode* node, Chunk* chunk) {
    switch (node->type) {
        case NODE_PRINT:
            generate_expression(node->data.print_statement.expression, chunk);
            write_chunk(chunk, OP_PRINT);
            break;
        case NODE_STATEMENTS: {
            ASTNode* current = node->data.statements.statement;
            while (current) {
                generate_statement(current, chunk);
                current = current->next;
            }
            break;
        }
        default:
            break; // Should not happen
    }
}

void generate_code(ASTNode* node, Chunk* chunk) {
    generate_statement(node, chunk);
    write_chunk(chunk, OP_RETURN);
}
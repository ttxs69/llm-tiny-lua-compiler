#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void debug_log(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

const char* node_type_to_string(NodeType type) {
    switch (type) {
        case NODE_NUMBER: return "NODE_NUMBER";
        case NODE_STRING: return "NODE_STRING";
        case NODE_IDENTIFIER: return "NODE_IDENTIFIER";
        case NODE_BINARY_OP: return "NODE_BINARY_OP";
        case NODE_UNARY_OP: return "NODE_UNARY_OP";
        case NODE_LOGICAL_OP: return "NODE_LOGICAL_OP";
        case NODE_ASSIGN: return "NODE_ASSIGN";
        case NODE_PRINT: return "NODE_PRINT";
        case NODE_IF: return "NODE_IF";
        case NODE_WHILE: return "NODE_WHILE";
        case NODE_STATEMENTS: return "NODE_STATEMENTS";
        case NODE_EXPRESSION_STATEMENT: return "NODE_EXPRESSION_STATEMENT";
        case NODE_FUNCTION_DEF: return "NODE_FUNCTION_DEF";
        case NODE_FUNCTION_CALL: return "NODE_FUNCTION_CALL";
        case NODE_RETURN: return "NODE_RETURN";
        case NODE_LOCAL_DECLARATION: return "NODE_LOCAL_DECLARATION";
        case NODE_TRUE: return "NODE_TRUE";
        case NODE_FALSE: return "NODE_FALSE";
        case NODE_NIL: return "NODE_NIL";
        default: return "UNKNOWN_NODE";
    }
}

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_EOF: return "TOKEN_EOF";
        case TOKEN_IDENTIFIER: return "TOKEN_IDENTIFIER";
        case TOKEN_NUMBER: return "TOKEN_NUMBER";
        case TOKEN_STRING: return "TOKEN_STRING";
        case TOKEN_PLUS: return "TOKEN_PLUS";
        case TOKEN_MINUS: return "TOKEN_MINUS";
        case TOKEN_MUL: return "TOKEN_MUL";
        case TOKEN_DIV: return "TOKEN_DIV";
        case TOKEN_LPAREN: return "TOKEN_LPAREN";
        case TOKEN_RPAREN: return "TOKEN_RPAREN";
        case TOKEN_COMMA: return "TOKEN_COMMA";
        case TOKEN_ASSIGN: return "TOKEN_ASSIGN";
        case TOKEN_PRINT: return "TOKEN_PRINT";
        case TOKEN_FUNCTION: return "TOKEN_FUNCTION";
        case TOKEN_IF: return "TOKEN_IF";
        case TOKEN_THEN: return "TOKEN_THEN";
        case TOKEN_ELSE: return "TOKEN_ELSE";
        case TOKEN_END: return "TOKEN_END";
        case TOKEN_GREATER: return "TOKEN_GREATER";
        case TOKEN_GREATER_EQUAL: return "TOKEN_GREATER_EQUAL";
        case TOKEN_LESS: return "TOKEN_LESS";
        case TOKEN_LESS_EQUAL: return "TOKEN_LESS_EQUAL";
        case TOKEN_EQUAL: return "TOKEN_EQUAL";
        case TOKEN_NOT_EQUAL: return "TOKEN_NOT_EQUAL";
        case TOKEN_WHILE: return "TOKEN_WHILE";
        case TOKEN_DO: return "TOKEN_DO";
        case TOKEN_NIL: return "TOKEN_NIL";
        case TOKEN_NOT: return "TOKEN_NOT";
        case TOKEN_CONCAT: return "TOKEN_CONCAT";
        case TOKEN_AND: return "TOKEN_AND";
        case TOKEN_OR: return "TOKEN_OR";
        case TOKEN_TRUE: return "TOKEN_TRUE";
        case TOKEN_FALSE: return "TOKEN_FALSE";
        case TOKEN_RETURN: return "TOKEN_RETURN";
        case TOKEN_LOCAL: return "TOKEN_LOCAL";
        case TOKEN_UNKNOWN: return "TOKEN_UNKNOWN";
        default: return "UNKNOWN_TOKEN";
    }
}

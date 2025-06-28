#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    NODE_NUMBER,
    NODE_STRING,
    NODE_IDENTIFIER,
    NODE_BINARY_OP,
    NODE_ASSIGN,
    NODE_PRINT,
    NODE_FUNCTION_CALL,
    NODE_STATEMENTS,
    NODE_EXPRESSION_STATEMENT
} NodeType;

typedef struct ASTNode {
    NodeType type;
    struct ASTNode *next; // For lists of statements
    union {
        // Number literal
        double number_value;
        // String literal
        char* string_value;
        // Identifier
        char* identifier_name;
        // Binary operation
        struct {
            struct ASTNode *left;
            TokenType op;
            struct ASTNode *right;
        } binary_op;
        // Assignment
        struct {
            char* identifier;
            struct ASTNode* expression;
        } assignment;
        // Print statement
        struct {
            struct ASTNode* expression;
        } print_statement;
        // Function call
        struct {
            char* function_name;
            struct ASTNode* argument;
        } function_call;
        // Statements
        struct {
            struct ASTNode* statement;
        } statements;
        // Expression statement
        struct {
            struct ASTNode* expression;
        } expression_statement;
    } data;
} ASTNode;

ASTNode* parse(const char* source);

#endif // PARSER_H
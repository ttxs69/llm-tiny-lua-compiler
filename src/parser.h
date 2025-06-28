#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    NODE_NUMBER,
    NODE_STRING,
    NODE_IDENTIFIER,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_LOGICAL_OP,
    NODE_PRINT,
    NODE_ASSIGN,
    NODE_IF,
    NODE_WHILE,
    NODE_STATEMENTS,
    NODE_EXPRESSION_STATEMENT,
    NODE_FUNCTION_DEF,
    NODE_FUNCTION_CALL,
    NODE_RETURN,
    NODE_LOCAL_DECLARATION,
    NODE_TRUE,
    NODE_FALSE,
    NODE_NIL
} NodeType;

typedef struct ASTNode {
    NodeType type;
    int line;
    struct ASTNode* next;
    union {
        double number_value;
        char* string_value;
        char* identifier_name;
        struct {
            TokenType op;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary_op;
        struct {
            TokenType op;
            struct ASTNode* right;
        } unary_op;
        struct {
            TokenType op;
            struct ASTNode* left;
            struct ASTNode* right;
        } logical_op;
        struct {
            struct ASTNode* expression;
        } print_statement;
        struct {
            char* identifier;
            struct ASTNode* expression;
        } assignment;
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_branch;
            struct ASTNode* else_branch;
        } if_statement;
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
        } while_statement;
        struct {
            struct ASTNode* statement;
        } statements;
        struct {
            struct ASTNode* expression;
        } expression_statement;
        struct {
            char* function_name;
            struct ASTNode* parameters;
            struct ASTNode* body;
        } function_def;
        struct {
            char* function_name;
            struct ASTNode* argument;
        } function_call;
        struct {
            struct ASTNode* expression;
        } return_statement;
        struct {
            char* identifier;
            struct ASTNode* expression;
        } local_declaration;
    } data;
} ASTNode;;

typedef struct {
    Token current;
    Token previous;
    int had_error;
    int panic_mode;
} Parser;

struct ASTNode* parse(const char* source);

#endif // PARSER_H

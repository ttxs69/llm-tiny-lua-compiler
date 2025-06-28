#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    NODE_NUMBER,
    NODE_STRING,
    NODE_IDENTIFIER,
    NODE_BINARY_OP,
    NODE_PRINT,
    NODE_ASSIGN,
    NODE_IF,
    NODE_WHILE,
    NODE_STATEMENTS,
    NODE_EXPRESSION_STATEMENT,
    NODE_FUNCTION_CALL,
    NODE_UNKNOWN
} NodeType;

struct ASTNode;

typedef struct {
    char* identifier;
    struct ASTNode* expression;
} Assignment;

typedef struct {
    struct ASTNode* condition;
    struct ASTNode* then_branch;
    struct ASTNode* else_branch;
} IfStatement;

typedef struct {
    struct ASTNode* condition;
    struct ASTNode* body;
} WhileStatement;

typedef struct {
    struct ASTNode* expression;
} PrintStatement;

typedef struct {
    struct ASTNode* expression;
} ExpressionStatement;

typedef struct {
    struct ASTNode* statement;
} Statements;

typedef struct {
    char* function_name;
    struct ASTNode* argument;
} FunctionCall;

typedef struct ASTNode {
    NodeType type;
    int line;
    struct ASTNode *next; // For lists of statements
    union {
        double number_value;
        char* string_value;
        char* identifier_name;
        struct {
            struct ASTNode *left;
            TokenType op;
            struct ASTNode *right;
        } binary_op;
        Assignment assignment;
        IfStatement if_statement;
        WhileStatement while_statement;
        PrintStatement print_statement;
        ExpressionStatement expression_statement;
        Statements statements;
        FunctionCall function_call;
    } data;
} ASTNode;

typedef struct {
    Token current;
    Token previous;
    int had_error;
    int panic_mode;
} Parser;

ASTNode* parse(const char* source);

#endif // PARSER_H
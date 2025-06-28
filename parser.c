#include "parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static Token current_token;
static Token previous_token;

static void advance() {
    previous_token = current_token;
    current_token = next_token();
}

static int check(TokenType type) {
    return current_token.type == type;
}

static int match(TokenType type) {
    if (check(type)) {
        advance();
        return 1;
    }
    return 0;
}

static ASTNode* expression(); // Forward declaration
static ASTNode* statement();


static ASTNode* primary() {
    if (match(TOKEN_NUMBER)) {
        ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
        node->type = NODE_NUMBER;
        char* num_str = (char*)malloc(previous_token.length + 1);
        memcpy(num_str, previous_token.start, previous_token.length);
        num_str[previous_token.length] = '\0';
        node->data.number_value = strtod(num_str, NULL);
        free(num_str);
        return node;
    }

    if (match(TOKEN_STRING)) {
        ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
        node->type = NODE_STRING;
        node->data.string_value = (char*)malloc(previous_token.length + 1);
        memcpy(node->data.string_value, previous_token.start, previous_token.length);
        node->data.string_value[previous_token.length] = '\0';
        return node;
    }

    if (match(TOKEN_IDENTIFIER)) {
        // Lookahead to see if it's a function call
        if (!is_at_end() && check(TOKEN_LPAREN)) {
            ASTNode* call_node = (ASTNode*)malloc(sizeof(ASTNode));
            call_node->type = NODE_FUNCTION_CALL;
            call_node->data.function_call.function_name = (char*)malloc(previous_token.length + 1);
            memcpy(call_node->data.function_call.function_name, previous_token.start, previous_token.length);
            call_node->data.function_call.function_name[previous_token.length] = '\0';
            
            match(TOKEN_LPAREN);
            call_node->data.function_call.argument = expression();
            match(TOKEN_RPAREN);
            return call_node;
        }

        ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
        node->type = NODE_IDENTIFIER;
        node->data.identifier_name = (char*)malloc(previous_token.length + 1);
        memcpy(node->data.identifier_name, previous_token.start, previous_token.length);
        node->data.identifier_name[previous_token.length] = '\0';
        return node;
    }
    
    if (match(TOKEN_LPAREN)) {
        ASTNode* expr = expression();
        if (!match(TOKEN_RPAREN)) {
            // Handle error
        }
        return expr;
    }


    return NULL; // Should not happen in valid code
}

static ASTNode* term() {
    ASTNode* node = primary();

    while (match(TOKEN_MUL) || match(TOKEN_DIV)) {
        ASTNode* new_node = (ASTNode*)malloc(sizeof(ASTNode));
        new_node->type = NODE_BINARY_OP;
        new_node->data.binary_op.op = previous_token.type;
        new_node->data.binary_op.left = node;
        new_node->data.binary_op.right = primary();
        node = new_node;
    }

    return node;
}

static ASTNode* expression() {
    ASTNode* node = term();

    while (match(TOKEN_PLUS) || match(TOKEN_MINUS)) {
        ASTNode* new_node = (ASTNode*)malloc(sizeof(ASTNode));
        new_node->type = NODE_BINARY_OP;
        new_node->data.binary_op.op = previous_token.type;
        new_node->data.binary_op.left = node;
        new_node->data.binary_op.right = term();
        node = new_node;
    }

    return node;
}

static ASTNode* statement() {
    if (match(TOKEN_PRINT)) {
        match(TOKEN_LPAREN);
        ASTNode* expr = expression();
        match(TOKEN_RPAREN);

        ASTNode* print_node = (ASTNode*)malloc(sizeof(ASTNode));
        print_node->type = NODE_PRINT;
        print_node->data.print_statement.expression = expr;
        return print_node;
    }

    if (check(TOKEN_IDENTIFIER)) {
        Token identifier_token = current_token;
        advance();
        if (match(TOKEN_ASSIGN)) {
            ASTNode* expr = expression();
            ASTNode* assign_node = (ASTNode*)malloc(sizeof(ASTNode));
            assign_node->type = NODE_ASSIGN;
            assign_node->data.assignment.identifier = (char*)malloc(identifier_token.length + 1);
            memcpy(assign_node->data.assignment.identifier, identifier_token.start, identifier_token.length);
            assign_node->data.assignment.identifier[identifier_token.length] = '\0';
            assign_node->data.assignment.expression = expr;
            return assign_node;
        }
        // Backtrack if it wasn't an assignment
        current_token = identifier_token;
        previous_token = (Token){.type = TOKEN_UNKNOWN}; // Reset previous
    }
    
    ASTNode* expr_node = expression();
    if (expr_node) {
        ASTNode* stmt_node = (ASTNode*)malloc(sizeof(ASTNode));
        stmt_node->type = NODE_EXPRESSION_STATEMENT;
        stmt_node->data.expression_statement.expression = expr_node;
        return stmt_node;
    }

    return NULL;
}


ASTNode* parse(const char* source) {
    init_lexer(source);
    advance();

    ASTNode* head = NULL;
    ASTNode* tail = NULL;

    while(!check(TOKEN_EOF)) {
        ASTNode* st = statement();
        if (st) {
            if (head == NULL) {
                head = st;
                tail = st;
            } else {
                tail->next = st;
                tail = st;
            }
        } else {
            break;
        }
    }

    ASTNode* root = (ASTNode*)malloc(sizeof(ASTNode));
    root->type = NODE_STATEMENTS;
    root->data.statements.statement = head;
    return root;
}

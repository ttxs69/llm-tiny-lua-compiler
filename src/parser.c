#include "parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static ASTNode* create_node(NodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->next = NULL;
    return node;
}

/**
 * @brief The parser state.
 */
static Parser parser;

/**
 * @brief Reports an error at the given token.
 * 
 * @param token The token where the error occurred.
 * @param message The error message.
 */
static void error_at(Token* token, const char* message) {
    if (parser.panic_mode) return;
    parser.panic_mode = 1;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_UNKNOWN) {
        // Nothing
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.had_error = 1;
}

/**
 * @brief Reports an error at the previous token.
 * 
 * @param message The error message.
 */
static void error(const char* message) {
    error_at(&parser.previous, message);
}

/**
 * @brief Reports an error at the current token.
 * 
 * @param message The error message.
 */
static void error_at_current(const char* message) {
    error_at(&parser.current, message);
}

/**
 * @brief Consumes the current token and advances to the next one.
 */
static void advance() {
    parser.previous = parser.current;
    parser.current = next_token();
    if (parser.current.type == TOKEN_UNKNOWN) {
        error_at_current("Unexpected character.");
    }
}

/**
 * @brief Checks if the current token has the given type.
 * 
 * @param type The type to check for.
 * @return 1 if the current token has the given type, 0 otherwise.
 */
static int check(TokenType type) {
    return parser.current.type == type;
}

/**
 * @brief If the current token has the given type, consumes it and returns 1.
 * Otherwise, returns 0.
 * 
 * @param type The type to match.
 * @return 1 if the token was matched, 0 otherwise.
 */
static int match(TokenType type) {
    if (check(type)) {
        advance();
        return 1;
    }
    return 0;
}

/**
 * @brief Consumes the current token if it has the given type. Otherwise,
 * reports an error.
 * 
 * @param type The type to consume.
 * @param message The error message to report if the token cannot be consumed.
 */
static void consume(TokenType type, const char* message) {
    if (check(type)) {
        advance();
        return;
    }
    error_at_current(message);
}

// Forward declarations for the parsing functions.
static ASTNode* expression();
static ASTNode* statement();

/**
 * @brief Parses a primary expression.
 * 
 * primary -> NUMBER | STRING | IDENTIFIER | "(" expression ")"
 * 
 * @return The parsed AST node.
 */
static ASTNode* primary() {
    if (match(TOKEN_NUMBER)) {
        ASTNode* node = create_node(NODE_NUMBER);
        node->line = parser.previous.line;
        char* num_str = (char*)malloc(parser.previous.length + 1);
        memcpy(num_str, parser.previous.start, parser.previous.length);
        num_str[parser.previous.length] = '\0';
        node->data.number_value = strtod(num_str, NULL);
        free(num_str);
        return node;
    }

    if (match(TOKEN_STRING)) {
        ASTNode* node = create_node(NODE_STRING);
        node->line = parser.previous.line;
        node->data.string_value = (char*)malloc(parser.previous.length - 1);
        memcpy(node->data.string_value, parser.previous.start + 1, parser.previous.length - 2);
        node->data.string_value[parser.previous.length - 2] = '\0';
        return node;
    }

    if (match(TOKEN_IDENTIFIER)) {
        // Lookahead to see if it's a function call
        if (!is_at_end() && check(TOKEN_LPAREN)) {
            ASTNode* call_node = create_node(NODE_FUNCTION_CALL);
            call_node->line = parser.previous.line;
            call_node->data.function_call.function_name = (char*)malloc(parser.previous.length + 1);
            memcpy(call_node->data.function_call.function_name, parser.previous.start, parser.previous.length);
            call_node->data.function_call.function_name[parser.previous.length] = '\0';
            
            match(TOKEN_LPAREN);
            call_node->data.function_call.argument = expression();
            consume(TOKEN_RPAREN, "Expect ')' after arguments.");
            return call_node;
        }

        ASTNode* node = create_node(NODE_IDENTIFIER);
        node->line = parser.previous.line;
        node->data.identifier_name = (char*)malloc(parser.previous.length + 1);
        memcpy(node->data.identifier_name, parser.previous.start, parser.previous.length);
        node->data.identifier_name[parser.previous.length] = '\0';
        return node;
    }
    
    if (match(TOKEN_LPAREN)) {
        ASTNode* expr = expression();
        consume(TOKEN_RPAREN, "Expect ')' after expression.");
        return expr;
    }

    error("Expect expression.");
    return NULL; // Should not happen in valid code
}

/**
 * @brief Parses a term.
 * 
 * term -> primary ( ( "*" | "/" ) primary )*
 * 
 * @return The parsed AST node.
 */
static ASTNode* term() {
    ASTNode* node = primary();

    while (match(TOKEN_MUL) || match(TOKEN_DIV)) {
        ASTNode* new_node = create_node(NODE_BINARY_OP);
        new_node->line = parser.previous.line;
        new_node->data.binary_op.op = parser.previous.type;
        new_node->data.binary_op.left = node;
        new_node->data.binary_op.right = primary();
        node = new_node;
    }

    return node;
}

/**
 * @brief Parses an expression.
 * 
 * expression -> term ( ( "+" | "-" | ">" | "<" ) term )*
 * 
 * @return The parsed AST node.
 */
static ASTNode* expression() {
    ASTNode* node = term();

    while (match(TOKEN_PLUS) || match(TOKEN_MINUS) || match(TOKEN_GREATER) || match(TOKEN_LESS)) {
        ASTNode* new_node = create_node(NODE_BINARY_OP);
        new_node->line = parser.previous.line;
        new_node->data.binary_op.op = parser.previous.type;
        new_node->data.binary_op.left = node;
        new_node->data.binary_op.right = term();
        node = new_node;
    }

    return node;
}

/**
 * @brief Parses an if statement.
 * 
 * ifStatement -> "if" expression "then" statement* ( "else" statement* )? "end"
 * 
 * @return The parsed AST node.
 */
static ASTNode* if_statement() {
    ASTNode* node = create_node(NODE_IF);
    node->line = parser.previous.line;

    node->data.if_statement.condition = expression();
    consume(TOKEN_THEN, "Expect 'then' after if condition.");

    ASTNode* then_branch = create_node(NODE_STATEMENTS);
    then_branch->line = parser.previous.line;
    then_branch->data.statements.statement = NULL;
    ASTNode* tail = NULL;

    while (!check(TOKEN_ELSE) && !check(TOKEN_END) && !check(TOKEN_EOF)) {
        ASTNode* st = statement();
        if (st) {
            if (then_branch->data.statements.statement == NULL) {
                then_branch->data.statements.statement = st;
                tail = st;
            } else {
                tail->next = st;
                tail = st;
            }
        } else {
            break;
        }
    }
    node->data.if_statement.then_branch = then_branch;

    if (match(TOKEN_ELSE)) {
        ASTNode* else_branch = create_node(NODE_STATEMENTS);
        else_branch->line = parser.previous.line;
        else_branch->data.statements.statement = NULL;
        tail = NULL;

        while (!check(TOKEN_END) && !check(TOKEN_EOF)) {
            ASTNode* st = statement();
            if (st) {
                if (else_branch->data.statements.statement == NULL) {
                    else_branch->data.statements.statement = st;
                    tail = st;
                } else {
                    tail->next = st;
                    tail = st;
                }
            } else {
                break;
            }
        }
        node->data.if_statement.else_branch = else_branch;
    } else {
        node->data.if_statement.else_branch = NULL;
    }

    consume(TOKEN_END, "Expect 'end' after if branches.");
    return node;
}

/**
 * @brief Parses a while statement.
 * 
 * whileStatement -> "while" expression "do" statement* "end"
 * 
 * @return The parsed AST node.
 */
static ASTNode* while_statement() {
    ASTNode* node = create_node(NODE_WHILE);
    node->line = parser.previous.line;

    node->data.while_statement.condition = expression();
    consume(TOKEN_DO, "Expect 'do' after while condition.");

    ASTNode* body = create_node(NODE_STATEMENTS);
    body->line = parser.previous.line;
    body->data.statements.statement = NULL;
    ASTNode* tail = NULL;

    while (!check(TOKEN_END) && !check(TOKEN_EOF)) {
        ASTNode* st = statement();
        if (st) {
            if (body->data.statements.statement == NULL) {
                body->data.statements.statement = st;
                tail = st;
            } else {
                tail->next = st;
                tail = st;
            }
        } else {
            break;
        }
    }
    node->data.while_statement.body = body;

    consume(TOKEN_END, "Expect 'end' after while body.");
    return node;
}

/**
 * @brief Parses a statement.
 * 
 * statement -> printStatement | ifStatement | whileStatement | assignment | expressionStatement
 * 
 * @return The parsed AST node.
 */
static ASTNode* statement() {
    if (match(TOKEN_PRINT)) {
        consume(TOKEN_LPAREN, "Expect '(' after 'print'.");
        ASTNode* expr = expression();
        consume(TOKEN_RPAREN, "Expect ')' after expression.");

        ASTNode* print_node = create_node(NODE_PRINT);
        print_node->line = parser.previous.line;
        print_node->data.print_statement.expression = expr;
        return print_node;
    }

    if (match(TOKEN_IF)) {
        return if_statement();
    }

    if (match(TOKEN_WHILE)) {
        return while_statement();
    }

    if (check(TOKEN_IDENTIFIER)) {
        Token identifier_token = parser.current;
        advance();
        if (match(TOKEN_ASSIGN)) {
            ASTNode* expr = expression();
            ASTNode* assign_node = create_node(NODE_ASSIGN);
            assign_node->line = identifier_token.line;
            assign_node->data.assignment.identifier = (char*)malloc(identifier_token.length + 1);
            memcpy(assign_node->data.assignment.identifier, identifier_token.start, identifier_token.length);
            assign_node->data.assignment.identifier[identifier_token.length] = '\0';
            assign_node->data.assignment.expression = expr;
            return assign_node;
        }
        // Backtrack if it wasn't an assignment
        parser.current = identifier_token;
        parser.previous = (Token){.type = TOKEN_UNKNOWN}; // Reset previous
    }
    
    ASTNode* expr_node = expression();
    if (expr_node) {
        ASTNode* stmt_node = create_node(NODE_EXPRESSION_STATEMENT);
        stmt_node->line = expr_node->line;
        stmt_node->data.expression_statement.expression = expr_node;
        return stmt_node;
    }

    return NULL;
}


/**
 * @brief Parses the given source code.
 * 
 * @param source The source code to parse.
 * @return The root of the AST, or NULL if there were errors.
 */
ASTNode* parse(const char* source) {
    init_lexer(source);
    parser.had_error = 0;
    parser.panic_mode = 0;
    advance();

    ASTNode* head = NULL;
    ASTNode* tail = NULL;

    while(!check(TOKEN_EOF)) {
        if (parser.panic_mode) {
            // TODO: Synchronize
        }
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

    if (parser.had_error) {
        // TODO: Free memory
        return NULL;
    }

    ASTNode* root = create_node(NODE_STATEMENTS);
    root->line = 0;
    root->data.statements.statement = head;
    return root;
}
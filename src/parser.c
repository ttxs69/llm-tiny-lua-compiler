#include "parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static struct ASTNode* create_node(NodeType type) {
    struct ASTNode* node = (struct ASTNode*)malloc(sizeof(struct ASTNode));
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
static struct ASTNode* expression();
static struct ASTNode* statement();
static struct ASTNode* primary();
static struct ASTNode* unary();
static struct ASTNode* term();
static struct ASTNode* and_expression();
static struct ASTNode* or_expression();
static struct ASTNode* if_statement();
static struct ASTNode* while_statement();
static struct ASTNode* function_declaration();
static struct ASTNode* return_statement();

/**
 * @brief Parses a primary expression.
 *
 * primary -> NUMBER | STRING | IDENTIFIER | "(" expression ")"
 *
 * @return The parsed AST node.
 */
static struct ASTNode* primary() {
    if (match(TOKEN_NUMBER)) {
        struct ASTNode* node = create_node(NODE_NUMBER);
        node->line = parser.previous.line;
        char* num_str = (char*)malloc(parser.previous.length + 1);
        memcpy(num_str, parser.previous.start, parser.previous.length);
        num_str[parser.previous.length] = '\0';
        node->data.number_value = strtod(num_str, NULL);
        free(num_str);
        return node;
    }

    if (match(TOKEN_STRING)) {
        struct ASTNode* node = create_node(NODE_STRING);
        node->line = parser.previous.line;
        node->data.string_value = (char*)malloc(parser.previous.length - 1);
        memcpy(node->data.string_value, parser.previous.start + 1, parser.previous.length - 2);
        node->data.string_value[parser.previous.length - 2] = '\0';
        return node;
    }

    if (match(TOKEN_IDENTIFIER)) {
        // Lookahead to see if it's a function call
        if (!is_at_end() && check(TOKEN_LPAREN)) {
            struct ASTNode* call_node = create_node(NODE_FUNCTION_CALL);
            call_node->line = parser.previous.line;
            call_node->data.function_call.function_name = (char*)malloc(parser.previous.length + 1);
            memcpy(call_node->data.function_call.function_name, parser.previous.start, parser.previous.length);
            call_node->data.function_call.function_name[parser.previous.length] = '\0';
            
            consume(TOKEN_LPAREN, "Expect '(' after function name.");

            struct ASTNode* args_head = NULL;
            struct ASTNode* args_tail = NULL;
            if (!check(TOKEN_RPAREN)) {
                do {
                    struct ASTNode* arg_node = expression();
                    if (args_head == NULL) {
                        args_head = arg_node;
                        args_tail = arg_node;
                    } else {
                        args_tail->next = arg_node;
                        args_tail = arg_node;
                    }
                } while (match(TOKEN_COMMA));
            }
            
            call_node->data.function_call.argument = args_head;
            consume(TOKEN_RPAREN, "Expect ')' after arguments.");
            return call_node;
        }

        struct ASTNode* node = create_node(NODE_IDENTIFIER);
        node->line = parser.previous.line;
        node->data.identifier_name = (char*)malloc(parser.previous.length + 1);
        memcpy(node->data.identifier_name, parser.previous.start, parser.previous.length);
        node->data.identifier_name[parser.previous.length] = '\0';
        return node;
    }
    
    if (match(TOKEN_LPAREN)) {
        struct ASTNode* expr = expression();
        consume(TOKEN_RPAREN, "Expect ')' after expression.");
        return expr;
    }

    if (match(TOKEN_TRUE)) {
        return create_node(NODE_TRUE);
    }

    if (match(TOKEN_FALSE)) {
        return create_node(NODE_FALSE);
    }

    if (match(TOKEN_NIL)) {
        return create_node(NODE_NIL);
    }

    error("Expect expression.");
    return NULL; // Should not happen in valid code
}

static struct ASTNode* unary() {
    if (match(TOKEN_NOT) || match(TOKEN_MINUS)) {
        struct ASTNode* node = create_node(NODE_UNARY_OP);
        node->line = parser.previous.line;
        node->data.unary_op.op = parser.previous.type;
        node->data.unary_op.right = unary();
        return node;
    }
    return primary();
}

/**
 * @brief Parses a term.
 *
 * term -> unary ( ( "*" | "/" ) unary )*
 *
 * @return The parsed AST node.
 */
static struct ASTNode* term() {
    struct ASTNode* node = unary();

    while (match(TOKEN_MUL) || match(TOKEN_DIV)) {
        struct ASTNode* new_node = create_node(NODE_BINARY_OP);
        new_node->line = parser.previous.line;
        new_node->data.binary_op.op = parser.previous.type;
        new_node->data.binary_op.left = node;
        new_node->data.binary_op.right = unary();
        node = new_node;
    }

    return node;
}

/**
 * @brief Parses an expression.
 *
 * expression -> term ( ( "+" | "-" | ">" | "<" | ">=" | "<=" | "==" | "~=" ) term )*
 *
 * @return The parsed AST node.
 */
static struct ASTNode* expression() {
    struct ASTNode* node = term();

    while (match(TOKEN_PLUS) || match(TOKEN_MINUS) || match(TOKEN_GREATER) || match(TOKEN_LESS) ||
           match(TOKEN_GREATER_EQUAL) || match(TOKEN_LESS_EQUAL) || match(TOKEN_EQUAL) || match(TOKEN_NOT_EQUAL)) {
        struct ASTNode* new_node = create_node(NODE_BINARY_OP);
        new_node->line = parser.previous.line;
        new_node->data.binary_op.op = parser.previous.type;
        new_node->data.binary_op.left = node;
        new_node->data.binary_op.right = term();
        node = new_node;
    }

    return node;
}

static struct ASTNode* and_expression() {
    struct ASTNode* node = expression();
    while (match(TOKEN_AND)) {
        struct ASTNode* new_node = create_node(NODE_BINARY_OP);
        new_node->line = parser.previous.line;
        new_node->data.binary_op.op = parser.previous.type;
        new_node->data.binary_op.left = node;
        new_node->data.binary_op.right = expression();
        node = new_node;
    }
    return node;
}

static struct ASTNode* or_expression() {
    struct ASTNode* node = and_expression();
    while (match(TOKEN_OR)) {
        struct ASTNode* new_node = create_node(NODE_BINARY_OP);
        new_node->line = parser.previous.line;
        new_node->data.binary_op.op = parser.previous.type;
        new_node->data.binary_op.left = node;
        new_node->data.binary_op.right = and_expression();
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
static struct ASTNode* if_statement() {
    struct ASTNode* node = create_node(NODE_IF);
    node->line = parser.previous.line;

    node->data.if_statement.condition = or_expression();
    consume(TOKEN_THEN, "Expect 'then' after if condition.");

    struct ASTNode* then_branch = create_node(NODE_STATEMENTS);
    then_branch->line = parser.previous.line;
    then_branch->data.statements.statement = NULL;
    struct ASTNode* tail = NULL;

    while (!check(TOKEN_ELSE) && !check(TOKEN_END) && !check(TOKEN_EOF)) {
        struct ASTNode* st = statement();
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
        struct ASTNode* else_branch = create_node(NODE_STATEMENTS);
        else_branch->line = parser.previous.line;
        else_branch->data.statements.statement = NULL;
        tail = NULL;

        while (!check(TOKEN_END) && !check(TOKEN_EOF)) {
            struct ASTNode* st = statement();
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
static struct ASTNode* while_statement() {
    struct ASTNode* node = create_node(NODE_WHILE);
    node->line = parser.previous.line;

    node->data.while_statement.condition = or_expression();
    consume(TOKEN_DO, "Expect 'do' after while condition.");

    struct ASTNode* body = create_node(NODE_STATEMENTS);
    body->line = parser.previous.line;
    body->data.statements.statement = NULL;
    struct ASTNode* tail = NULL;

    while (!check(TOKEN_END) && !check(TOKEN_EOF)) {
        struct ASTNode* st = statement();
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
static struct ASTNode* statement() {
    if (match(TOKEN_PRINT)) {
        consume(TOKEN_LPAREN, "Expect '(' after 'print'.");
        struct ASTNode* expr = or_expression();
        consume(TOKEN_RPAREN, "Expect ')' after expression.");

        struct ASTNode* print_node = create_node(NODE_PRINT);
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

    if (match(TOKEN_FUNCTION)) {
        return function_declaration();
    }

    if (match(TOKEN_RETURN)) {
        return return_statement();
    }

    if (check(TOKEN_IDENTIFIER)) {
        Token identifier_token = parser.current;
        advance();
        if (match(TOKEN_ASSIGN)) {
            struct ASTNode* expr = or_expression();
            struct ASTNode* assign_node = create_node(NODE_ASSIGN);
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
    
    struct ASTNode* expr_node = or_expression();
    if (expr_node) {
        struct ASTNode* stmt_node = create_node(NODE_EXPRESSION_STATEMENT);
        stmt_node->line = expr_node->line;
        stmt_node->data.expression_statement.expression = expr_node;
        return stmt_node;
    }

    return NULL;
}

static struct ASTNode* function_declaration() {
    struct ASTNode* node = create_node(NODE_FUNCTION_DEF);
    node->line = parser.previous.line;

    consume(TOKEN_IDENTIFIER, "Expect function name.");
    node->data.function_def.function_name = (char*)malloc(parser.previous.length + 1);
    memcpy(node->data.function_def.function_name, parser.previous.start, parser.previous.length);
    node->data.function_def.function_name[parser.previous.length] = '\0';

    consume(TOKEN_LPAREN, "Expect '(' after function name.");

    struct ASTNode* params_head = NULL;
    struct ASTNode* params_tail = NULL;
    if (!check(TOKEN_RPAREN)) {
        do {
            consume(TOKEN_IDENTIFIER, "Expect parameter name.");
            struct ASTNode* param_node = create_node(NODE_IDENTIFIER);
            param_node->line = parser.previous.line;
            param_node->data.identifier_name = (char*)malloc(parser.previous.length + 1);
            memcpy(param_node->data.identifier_name, parser.previous.start, parser.previous.length);
            param_node->data.identifier_name[parser.previous.length] = '\0';

            if (params_head == NULL) {
                params_head = param_node;
                params_tail = param_node;
            } else {
                params_tail->next = param_node;
                params_tail = param_node;
            }
        } while (match(TOKEN_COMMA));
    }
    consume(TOKEN_RPAREN, "Expect ')' after parameters.");

    node->data.function_def.parameters = params_head;

    struct ASTNode* body = create_node(NODE_STATEMENTS);
    body->line = parser.previous.line;
    body->data.statements.statement = NULL;
    struct ASTNode* tail = NULL;

    while (!check(TOKEN_END) && !check(TOKEN_EOF)) {
        struct ASTNode* st = statement();
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
    node->data.function_def.body = body;

    consume(TOKEN_END, "Expect 'end' after function body.");
    return node;
}

static struct ASTNode* return_statement() {
    struct ASTNode* node = create_node(NODE_RETURN);
    node->line = parser.previous.line;
    node->data.return_statement.expression = expression();
    return node;
}


/**
 * @brief Parses the given source code.
 * 
 * @param source The source code to parse.
 * @return The root of the AST, or NULL if there were errors.
 */
struct ASTNode* parse(const char* source) {
    init_lexer(source);
    parser.had_error = 0;
    parser.panic_mode = 0;
    advance();

    struct ASTNode* head = NULL;
    struct ASTNode* tail = NULL;

    while(!check(TOKEN_EOF)) {
        if (parser.panic_mode) {
            // TODO: Synchronize
        }
        struct ASTNode* st = statement();
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

    struct ASTNode* root = create_node(NODE_STATEMENTS);
    root->line = 0;
    root->data.statements.statement = head;
    return root;
}
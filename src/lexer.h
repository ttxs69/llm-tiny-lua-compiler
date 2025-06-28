#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_COMMA,
    TOKEN_ASSIGN,
    TOKEN_PRINT,
    TOKEN_FUNCTION,
    TOKEN_IF,
    TOKEN_THEN,
    TOKEN_ELSE,
    TOKEN_END,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_WHILE,
    TOKEN_DO,
    TOKEN_NIL,
    TOKEN_NOT,
    TOKEN_CONCAT,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_RETURN,
    TOKEN_LOCAL,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    const char *start;
    int length;
    int line;
} Token;

void init_lexer(const char *source);
Token next_token();
int is_at_end();

#endif // LEXER_H

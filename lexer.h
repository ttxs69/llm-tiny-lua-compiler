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
    TOKEN_ASSIGN,
    TOKEN_PRINT,
    TOKEN_FUNCTION,
    TOKEN_END,
    TOKEN_IF,
    TOKEN_THEN,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_DO,
    TOKEN_LOCAL,
    TOKEN_RETURN,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    const char *start;
    int length;
} Token;

void init_lexer(const char *source);
Token next_token();
int is_at_end();

#endif // LEXER_H

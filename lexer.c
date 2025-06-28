#include "lexer.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

static const char *source;

void init_lexer(const char *src) {
    source = src;
}

static Token make_token(TokenType type, int length) {
    Token token;
    token.type = type;
    token.start = source;
    token.length = length;
    return token;
}

static Token error_token() {
    Token token;
    token.type = TOKEN_UNKNOWN;
    token.start = "Error";
    token.length = 5;
    return token;
}

static char peek() { return *source; }
static char advance() { return *source++; }
int is_at_end() { return *source == '\0'; }

static void skip_whitespace() {
    while (1) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
            case '\n':
                advance();
                break;
            default:
                return;
        }
    }
}

static TokenType check_keyword(const char* start, int length, const char* rest, TokenType type) {
    if (source - start == strlen(rest) && length == strlen(rest) && memcmp(start, rest, length) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

static TokenType identifier_type(const char* start) {
    int length = source - start;
    switch (start[0]) {
        case 'p': return check_keyword(start, length, "print", TOKEN_PRINT);
        case 'f': return check_keyword(start, length, "function", TOKEN_FUNCTION);
        case 'e': 
            if (length == 3 && memcmp(start, "end", 3) == 0) return TOKEN_END;
            if (length == 4 && memcmp(start, "else", 4) == 0) return TOKEN_ELSE;
            break;
        case 'i': return check_keyword(start, length, "if", TOKEN_IF);
        case 't': return check_keyword(start, length, "then", TOKEN_THEN);
        case 'w': return check_keyword(start, length, "while", TOKEN_WHILE);
        case 'd': return check_keyword(start, length, "do", TOKEN_DO);
        case 'l': return check_keyword(start, length, "local", TOKEN_LOCAL);
        case 'r': return check_keyword(start, length, "return", TOKEN_RETURN);
    }
    return TOKEN_IDENTIFIER;
}


static Token identifier() {
    const char* start = source;
    while ((isalpha(peek()) || isdigit(peek()) || peek() == '_') && !is_at_end()) advance();
    return make_token(identifier_type(start), source - start);
}

static Token number() {
    const char* start = source;
    while (isdigit(peek())) advance();
    if (peek() == '.' && isdigit(source[1])) {
        advance();
        while (isdigit(peek())) advance();
    }
    return make_token(TOKEN_NUMBER, source - start);
}

static Token string() {
    const char* start = source + 1;
    while (peek() != '"' && !is_at_end()) {
        advance();
    }
    if (is_at_end()) return error_token();
    advance(); // Closing quote
    return make_token(TOKEN_STRING, source - start - 2);
}


Token next_token() {
    skip_whitespace();
    if (is_at_end()) return make_token(TOKEN_EOF, 0);

    char c = peek();
    if (isalpha(c) || c == '_') return identifier();
    if (isdigit(c)) return number();

    switch (c) {
        case '(': source++; return make_token(TOKEN_LPAREN, 1);
        case ')': source++; return make_token(TOKEN_RPAREN, 1);
        case '+': source++; return make_token(TOKEN_PLUS, 1);
        case '-': source++; return make_token(TOKEN_MINUS, 1);
        case '*': source++; return make_token(TOKEN_MUL, 1);
        case '/': source++; return make_token(TOKEN_DIV, 1);
        case '=': source++; return make_token(TOKEN_ASSIGN, 1);
        case '"': return string();
    }

    return error_token();
}

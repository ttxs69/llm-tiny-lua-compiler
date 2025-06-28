#include "lexer.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/**
 * @brief The source code being scanned.
 */
static const char *source;
/**
 * @brief The current line number in the source code.
 */
static int line;

/**
 * @brief Initializes the lexer with the given source code.
 *
 * @param src The source code to scan.
 */
void init_lexer(const char *src) {
    source = src;
    line = 1;
}

/**
 * @brief Creates a new token.
 *
 * @param type The type of the token.
 * @param start A pointer to the start of the token's lexeme.
 * @param length The length of the token's lexeme.
 * @return The new token.
 */
static Token make_token(TokenType type, const char* start, int length) {
    Token token;
    token.type = type;
    token.start = start;
    token.length = length;
    token.line = line;
    return token;
}

/**
 * @brief Creates a new error token.
 *
 * @return The new error token.
 */
static Token error_token() {
    Token token;
    token.type = TOKEN_UNKNOWN;
    token.start = "Error";
    token.length = 5;
    token.line = line;
    return token;
}

/**
 * @brief Peeks at the current character without consuming it.
 *
 * @return The current character.
 */
static char peek() { return *source; }
/**
 * @brief Consumes the current character and returns it.
 *
 * @return The consumed character.
 */
static char advance() { return *source++; }
/**
 * @brief Checks if the end of the source code has been reached.
 *
 * @return 1 if the end of the source code has been reached, 0 otherwise.
 */
int is_at_end() { return *source == '\0'; }

/**
 * @brief Skips whitespace characters.
 */
static void skip_whitespace() {
    while (1) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                line++;
                advance();
                break;
            default:
                return;
        }
    }
}

/**
 * @brief Checks if the given identifier is a keyword.
 *
 * @param start A pointer to the start of the identifier's lexeme.
 * @param length The length of the identifier's lexeme.
 * @param rest The rest of the keyword to check.
 * @param type The type of the keyword.
 * @return The type of the keyword if it matches, TOKEN_IDENTIFIER otherwise.
 */
static TokenType check_keyword(const char* start, int length, const char* rest, TokenType type) {
    if (source - start == strlen(rest) && length == strlen(rest) && memcmp(start, rest, length) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

/**
 * @brief Determines the type of an identifier.
 *
 * @param start A pointer to the start of the identifier's lexeme.
 * @return The type of the identifier.
 */
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


/**
 * @brief Scans an identifier.
 *
 * @return The scanned identifier token.
 */
static Token identifier() {
    const char* start = source;
    while ((isalpha(peek()) || isdigit(peek()) || peek() == '_') && !is_at_end()) advance();
    return make_token(identifier_type(start), start, source - start);
}

/**
 * @brief Scans a number.
 *
 * @return The scanned number token.
 */
static Token number() {
    const char* start = source;
    while (isdigit(peek())) advance();
    if (peek() == '.' && isdigit(source[1])) {
        advance();
        while (isdigit(peek())) advance();
    }
    return make_token(TOKEN_NUMBER, start, source - start);
}

/**
 * @brief Scans a string.
 *
 * @return The scanned string token.
 */
static Token string() {
    const char* start = source;
    advance(); // Opening quote
    while (peek() != '"' && !is_at_end()) {
        advance();
    }
    if (is_at_end()) return error_token();
    advance(); // Closing quote
    return make_token(TOKEN_STRING, start, source - start);
}


/**
 * @brief Scans the next token.
 *
 * @return The next token.
 */
Token next_token() {
    skip_whitespace();
    if (is_at_end()) return make_token(TOKEN_EOF, source, 0);

    char c = peek();
    if (isalpha(c) || c == '_') return identifier();
    if (isdigit(c)) return number();

    const char* start = source;
    switch (c) {
        case '(': source++; return make_token(TOKEN_LPAREN, start, 1);
        case ')': source++; return make_token(TOKEN_RPAREN, start, 1);
        case '+': source++; return make_token(TOKEN_PLUS, start, 1);
        case '-': source++; return make_token(TOKEN_MINUS, start, 1);
        case '*': source++; return make_token(TOKEN_MUL, start, 1);
        case '/': source++; return make_token(TOKEN_DIV, start, 1);
        case '=': source++; return make_token(TOKEN_ASSIGN, start, 1);
        case '>': source++; return make_token(TOKEN_GREATER, start, 1);
        case '<': source++; return make_token(TOKEN_LESS, start, 1);
        case '"': return string();
    }

    return error_token();
}

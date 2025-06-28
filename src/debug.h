#ifndef DEBUG_H
#define DEBUG_H

#include <stdarg.h>
#include "parser.h"
#include "lexer.h"

void debug_log(const char *format, ...);
const char* node_type_to_string(NodeType type);
const char* token_type_to_string(TokenType type);

#endif // DEBUG_H

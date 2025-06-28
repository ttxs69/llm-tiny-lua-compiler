#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"
#include "bytecode.h"

void generate_code(struct ASTNode* node, Chunk* chunk);

#endif // CODEGEN_H

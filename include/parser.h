// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#ifndef NADIR_PARSER_H
#define NADIR_PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct Parser {
    Lexer* lexer;
    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
} Parser;

void parser_init(Parser* parser, Lexer* lexer);
ASTNode* parser_parse(Parser* parser);

#endif // PARSER_H

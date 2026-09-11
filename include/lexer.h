// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#ifndef NADIR_LEXER_H
#define NADIR_LEXER_H

#include "token.h"

typedef struct {
    const char* source;
    const char* start;
    const char* current;
    int line;
    int column;
} Lexer;

void lexer_init(Lexer* lexer, const char* source);
Token lexer_next_token(Lexer* lexer);

#endif // LEXER_H

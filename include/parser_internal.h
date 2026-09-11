// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#ifndef NADIR_PARSER_INTERNAL_H
#define NADIR_PARSER_INTERNAL_H

#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void advance_token(Parser* parser);
bool check_token(Parser* parser, TokenType type);
bool match_token(Parser* parser, TokenType type);
void consume_token(Parser* parser, TokenType type, const char* message);

bool is_identifier_token(TokenType type);
char* consume_identifier_name(Parser* parser, const char* message);
bool is_type_token(TokenType type);
char* parse_type_name(Parser* parser);
void skip_annotations(Parser* parser);

ASTNode* parse_declaration(Parser* parser);
ASTNode* parse_class_declaration(Parser* parser);
ASTNode* parse_statement(Parser* parser);
ASTNode* parse_block(Parser* parser);

ASTNode* parse_expression(Parser* parser);
ASTNode* parse_assignment(Parser* parser);
ASTNode* parse_ternary(Parser* parser);
ASTNode* parse_logical_or(Parser* parser);
ASTNode* parse_null_coalescing(Parser* parser);
ASTNode* parse_logical_and(Parser* parser);
ASTNode* parse_equality(Parser* parser);
ASTNode* parse_relational(Parser* parser);
ASTNode* parse_additive(Parser* parser);
ASTNode* parse_multiplicative(Parser* parser);
ASTNode* parse_unary(Parser* parser);
ASTNode* parse_postfix(Parser* parser);
ASTNode* parse_primary(Parser* parser);

ASTNode* parse_soql_query(Parser* parser);

extern void ast_node_array_append(ASTNodeArray* arr, ASTNode* node);

#endif

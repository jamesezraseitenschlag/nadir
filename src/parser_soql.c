// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#include "parser_internal.h"

ASTNode* parse_soql_query(Parser* parser) {
    int line = parser->previous.line;
    consume_token(parser, TOKEN_KW_SELECT, "Expected 'SELECT' in SOQL query.");

    char* fields[32];
    int f_count = 0;

    while (!check_token(parser, TOKEN_KW_FROM) && !check_token(parser, TOKEN_RBRACKET) && !check_token(parser, TOKEN_EOF)) {
        if (check_token(parser, TOKEN_LPAREN)) {
            int pdepth = 1;
            advance_token(parser);
            while (pdepth > 0 && !check_token(parser, TOKEN_EOF)) {
                if (match_token(parser, TOKEN_LPAREN)) pdepth++;
                else if (match_token(parser, TOKEN_RPAREN)) pdepth--;
                else advance_token(parser);
            }
            continue;
        }
        if (is_identifier_token(parser->current.type)) {
            if (f_count < 30) {
                fields[f_count++] = consume_identifier_name(parser, "Expected field name.");
            } else {
                advance_token(parser);
            }
        } else {
            advance_token(parser);
        }
    }

    if (f_count == 0) {
        fields[f_count++] = duplicate_string("Id");
    }

    consume_token(parser, TOKEN_KW_FROM, "Expected 'FROM' in SOQL query.");
    char* from_obj = consume_identifier_name(parser, "Expected SObject name in FROM clause.");

    char* where_field = NULL;
    char* where_op = NULL;
    ASTNode* where_val = NULL;
    int limit = 0;

    int bracket_depth = 0;
    while ((!check_token(parser, TOKEN_RBRACKET) || bracket_depth > 0) && !check_token(parser, TOKEN_EOF)) {
        if (match_token(parser, TOKEN_LBRACKET)) {
            bracket_depth++;
            continue;
        }
        if (match_token(parser, TOKEN_RBRACKET)) {
            bracket_depth--;
            continue;
        }
        if (match_token(parser, TOKEN_KW_WHERE) && !where_field) {
            where_field = consume_identifier_name(parser, "Expected field name in WHERE clause.");
            if (match_token(parser, TOKEN_EQUAL) || match_token(parser, TOKEN_ASSIGN) ||
                match_token(parser, TOKEN_NOT_EQUAL) || match_token(parser, TOKEN_GT) || match_token(parser, TOKEN_LT)) {
                where_op = duplicate_slice(parser->previous.start, parser->previous.length);
            } else if (match_token(parser, TOKEN_KW_LIKE)) {
                where_op = duplicate_string("LIKE");
            } else {
                where_op = duplicate_string("=");
            }

            if (match_token(parser, TOKEN_COLON)) {
                char* bind_name = consume_identifier_name(parser, "Expected variable name after ':'.");
                ASTNode* bind_var = ast_new_node(NODE_IDENTIFIER, parser->previous.line);
                bind_var->as.identifier.name = bind_name;
                while (match_token(parser, TOKEN_DOT)) {
                    char* mem = consume_identifier_name(parser, "Expected field name after '.'.");
                    ASTNode* mem_node = ast_new_node(NODE_MEMBER_ACCESS, parser->previous.line);
                    mem_node->as.member_access.target = bind_var;
                    mem_node->as.member_access.member_name = mem;
                    mem_node->as.member_access.safe_nav = false;
                    bind_var = mem_node;
                }
                where_val = bind_var;
            } else {
                where_val = parse_primary(parser);
            }
            continue;
        }

        if (match_token(parser, TOKEN_KW_LIMIT)) {
            if (match_token(parser, TOKEN_INT_LITERAL)) {
                limit = atoi(parser->previous.start);
            }
            continue;
        }

        advance_token(parser);
    }

    consume_token(parser, TOKEN_RBRACKET, "Expected ']' at end of SOQL query.");

    ASTNode* soql = ast_new_node(NODE_SOQL, line);
    soql->as.soql.from_object = from_obj;
    soql->as.soql.fields = (char**)malloc(sizeof(char*) * f_count);
    memcpy(soql->as.soql.fields, fields, sizeof(char*) * f_count);
    soql->as.soql.field_count = f_count;
    soql->as.soql.where_field = where_field;
    soql->as.soql.where_op = where_op;
    soql->as.soql.where_val = where_val;
    soql->as.soql.limit = limit;
    return soql;
}

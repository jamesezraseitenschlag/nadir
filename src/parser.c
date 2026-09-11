// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#include "parser_internal.h"

void parser_init(Parser* parser, Lexer* lexer) {
    parser->lexer = lexer;
    parser->had_error = false;
    parser->panic_mode = false;
    parser->current = lexer_next_token(lexer);
}

void advance_token(Parser* parser) {
    parser->previous = parser->current;
    while (true) {
        parser->current = lexer_next_token(parser->lexer);
        if (parser->current.type != TOKEN_ERROR) break;

        fprintf(stderr, "[Line %d] Lexer Error: %.*s\n",
                parser->current.line, parser->current.length, parser->current.start);
        parser->had_error = true;
    }
}

bool check_token(Parser* parser, TokenType type) {
    return parser->current.type == type;
}

bool match_token(Parser* parser, TokenType type) {
    if (!check_token(parser, type)) return false;
    advance_token(parser);
    return true;
}

void consume_token(Parser* parser, TokenType type, const char* message) {
    if (parser->current.type == type) {
        advance_token(parser);
        return;
    }
    fprintf(stderr, "[Line %d] Parser Error at '%.*s': %s\n",
            parser->current.line, parser->current.length, parser->current.start, message);
    parser->had_error = true;
}

bool is_identifier_token(TokenType type) {
    if (type == TOKEN_IDENTIFIER) return true;
    if (type >= TOKEN_KW_CLASS && type <= TOKEN_KW_MAX) return true;
    return false;
}

char* consume_identifier_name(Parser* parser, const char* message) {
    if (is_identifier_token(parser->current.type)) {
        advance_token(parser);
        return duplicate_slice(parser->previous.start, parser->previous.length);
    }
    fprintf(stderr, "[Line %d] Parser Error at '%.*s': %s\n",
            parser->current.line, parser->current.length, parser->current.start, message);
    parser->had_error = true;
    return duplicate_string("error");
}

bool is_type_token(TokenType type) {
    switch (type) {
        case TOKEN_KW_INTEGER:
        case TOKEN_KW_DOUBLE:
        case TOKEN_KW_DECIMAL:
        case TOKEN_KW_LONG:
        case TOKEN_KW_STRING:
        case TOKEN_KW_BOOLEAN:
        case TOKEN_KW_DATE:
        case TOKEN_KW_DATETIME:
        case TOKEN_KW_TIME:
        case TOKEN_KW_BLOB:
        case TOKEN_KW_ID:
        case TOKEN_KW_OBJECT:
        case TOKEN_KW_VOID:
        case TOKEN_KW_LIST:
        case TOKEN_KW_MAP:
        case TOKEN_KW_SET:
            return true;
        default:
            return false;
    }
}

char* parse_type_name(Parser* parser) {
    char buf[256] = {0};
    int len = parser->current.length;
    if (len >= 240) len = 240;
    strncpy(buf, parser->current.start, len);
    advance_token(parser);

    while (match_token(parser, TOKEN_DOT)) {
        strcat(buf, ".");
        if (is_identifier_token(parser->current.type)) {
            strncat(buf, parser->current.start, parser->current.length);
            advance_token(parser);
        }
    }

    if (check_token(parser, TOKEN_LT)) {
        const char* saved_start = parser->lexer->start;
        const char* saved_cur = parser->lexer->current;
        int saved_line = parser->lexer->line;
        int saved_col = parser->lexer->column;
        Token saved_current = parser->current;
        Token saved_prev = parser->previous;

        advance_token(parser);
        if (is_type_token(parser->current.type) || is_identifier_token(parser->current.type)) {
            char* inner = parse_type_name(parser);
            char generic_buf[256] = "<";
            strcat(generic_buf, inner);
            free(inner);
            bool valid_generic = true;
            while (match_token(parser, TOKEN_COMMA)) {
                strcat(generic_buf, ", ");
                char* next_inner = parse_type_name(parser);
                strcat(generic_buf, next_inner);
                free(next_inner);
            }
            if (match_token(parser, TOKEN_GT)) {
                strcat(generic_buf, ">");
                strcat(buf, generic_buf);
            } else {
                valid_generic = false;
            }

            if (!valid_generic) {
                parser->lexer->start = saved_start;
                parser->lexer->current = saved_cur;
                parser->lexer->line = saved_line;
                parser->lexer->column = saved_col;
                parser->current = saved_current;
                parser->previous = saved_prev;
            }
        } else {
            parser->lexer->start = saved_start;
            parser->lexer->current = saved_cur;
            parser->lexer->line = saved_line;
            parser->lexer->column = saved_col;
            parser->current = saved_current;
            parser->previous = saved_prev;
        }
    }

    while (check_token(parser, TOKEN_LBRACKET)) {
        const char* saved_start = parser->lexer->start;
        const char* saved_cur = parser->lexer->current;
        int saved_line = parser->lexer->line;
        int saved_col = parser->lexer->column;
        Token saved_current = parser->current;
        Token saved_prev = parser->previous;

        advance_token(parser);
        if (match_token(parser, TOKEN_RBRACKET)) {
            strcat(buf, "[]");
        } else {
            parser->lexer->start = saved_start;
            parser->lexer->current = saved_cur;
            parser->lexer->line = saved_line;
            parser->lexer->column = saved_col;
            parser->current = saved_current;
            parser->previous = saved_prev;
            break;
        }
    }

    return duplicate_string(buf);
}

void skip_annotations(Parser* parser) {
    while (match_token(parser, TOKEN_AT)) {
        consume_identifier_name(parser, "Expected annotation name.");
        if (match_token(parser, TOKEN_LPAREN)) {
            int depth = 1;
            while (depth > 0 && !check_token(parser, TOKEN_EOF)) {
                if (match_token(parser, TOKEN_LPAREN)) depth++;
                else if (match_token(parser, TOKEN_RPAREN)) depth--;
                else advance_token(parser);
            }
        }
    }
}

ASTNode* parser_parse(Parser* parser) {
    ASTNode* program = ast_new_node(NODE_PROGRAM, 1);
    while (!match_token(parser, TOKEN_EOF)) {
        ASTNode* decl = parse_declaration(parser);
        if (decl) {
            ast_node_array_append(&program->as.program.statements, decl);
        }
    }
    return program;
}

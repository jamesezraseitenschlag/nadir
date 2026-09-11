// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#include "lexer.h"
#include "common.h"

void lexer_init(Lexer* lexer, const char* source) {
    // skip utf8 bom
    if ((unsigned char)source[0] == 0xEF && (unsigned char)source[1] == 0xBB && (unsigned char)source[2] == 0xBF) {
        source += 3;
    }
    lexer->source = source;
    lexer->start = source;
    lexer->current = source;
    lexer->line = 1;
    lexer->column = 1;
}

static bool is_at_end(Lexer* lexer) {
    return *lexer->current == '\0';
}

static char advance(Lexer* lexer) {
    char c = *lexer->current++;
    if (c == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    return c;
}

static char peek(Lexer* lexer) {
    return *lexer->current;
}

static char peek_next(Lexer* lexer) {
    if (is_at_end(lexer)) return '\0';
    return lexer->current[1];
}

static bool match(Lexer* lexer, char expected) {
    if (is_at_end(lexer)) return false;
    if (*lexer->current != expected) return false;
    advance(lexer);
    return true;
}

static Token make_token(Lexer* lexer, TokenType type) {
    Token token;
    token.type = type;
    token.start = lexer->start;
    token.length = (int)(lexer->current - lexer->start);
    token.line = lexer->line;
    token.column = lexer->column - token.length;
    return token;
}

static Token error_token(Lexer* lexer, const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = lexer->line;
    token.column = lexer->column;
    return token;
}

static void skip_whitespace_and_comments(Lexer* lexer) {
    while (!is_at_end(lexer)) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance(lexer);
                break;
            case '\n':
                advance(lexer);
                break;
            case '/':
                if (peek_next(lexer) == '/') {
                    // single line comment
                    while (peek(lexer) != '\n' && !is_at_end(lexer)) advance(lexer);
                } else if (peek_next(lexer) == '*') {
                    // block comment
                    advance(lexer);
                    advance(lexer);
                    while (!is_at_end(lexer)) {
                        if (peek(lexer) == '*' && peek_next(lexer) == '/') {
                            advance(lexer);
                            advance(lexer);
                            break;
                        }
                        advance(lexer);
                    }
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}



static TokenType identifier_type(Lexer* lexer) {
    const char* s = lexer->start;
    int len = (int)(lexer->current - lexer->start);

    switch (tolower((unsigned char)s[0])) {
        case 'a':
            if (len == 8 && STRNCASECMP(s, "abstract", 8) == 0) return TOKEN_KW_ABSTRACT;
            if (len == 5 && STRNCASECMP(s, "after", 5) == 0) return TOKEN_KW_AFTER;
            if (len == 3 && STRNCASECMP(s, "and", 3) == 0) return TOKEN_KW_AND_SOQL;
            if (len == 3 && STRNCASECMP(s, "asc", 3) == 0) return TOKEN_KW_ASC;
            if (len == 3 && STRNCASECMP(s, "avg", 3) == 0) return TOKEN_KW_AVG;
            break;
        case 'b':
            if (len == 6 && STRNCASECMP(s, "before", 6) == 0) return TOKEN_KW_BEFORE;
            if (len == 7 && STRNCASECMP(s, "boolean", 7) == 0) return TOKEN_KW_BOOLEAN;
            if (len == 4 && STRNCASECMP(s, "blob", 4) == 0) return TOKEN_KW_BLOB;
            if (len == 5 && STRNCASECMP(s, "break", 5) == 0) return TOKEN_KW_BREAK;
            if (len == 2 && STRNCASECMP(s, "by", 2) == 0) return TOKEN_KW_BY;
            break;
        case 'c':
            if (len == 5 && STRNCASECMP(s, "class", 5) == 0) return TOKEN_KW_CLASS;
            if (len == 5 && STRNCASECMP(s, "catch", 5) == 0) return TOKEN_KW_CATCH;
            if (len == 8 && STRNCASECMP(s, "continue", 8) == 0) return TOKEN_KW_CONTINUE;
            if (len == 5 && STRNCASECMP(s, "count", 5) == 0) return TOKEN_KW_COUNT;
            break;
        case 'd':
            if (len == 4 && STRNCASECMP(s, "date", 4) == 0) return TOKEN_KW_DATE;
            if (len == 8 && STRNCASECMP(s, "datetime", 8) == 0) return TOKEN_KW_DATETIME;
            if (len == 7 && STRNCASECMP(s, "decimal", 7) == 0) return TOKEN_KW_DECIMAL;
            if (len == 6 && STRNCASECMP(s, "double", 6) == 0) return TOKEN_KW_DOUBLE;
            if (len == 6 && STRNCASECMP(s, "delete", 6) == 0) return TOKEN_KW_DML_DELETE;
            if (len == 4 && STRNCASECMP(s, "desc", 4) == 0) return TOKEN_KW_DESC;
            if (len == 2 && STRNCASECMP(s, "do", 2) == 0) return TOKEN_KW_DO;
            break;
        case 'e':
            if (len == 4 && STRNCASECMP(s, "else", 4) == 0) return TOKEN_KW_ELSE;
            if (len == 4 && STRNCASECMP(s, "enum", 4) == 0) return TOKEN_KW_ENUM;
            if (len == 7 && STRNCASECMP(s, "extends", 7) == 0) return TOKEN_KW_EXTENDS;
            break;
        case 'f':
            if (len == 5 && STRNCASECMP(s, "false", 5) == 0) return TOKEN_BOOL_LITERAL;
            if (len == 5 && STRNCASECMP(s, "final", 5) == 0) return TOKEN_KW_FINAL;
            if (len == 7 && STRNCASECMP(s, "finally", 7) == 0) return TOKEN_KW_FINALLY;
            if (len == 3 && STRNCASECMP(s, "for", 3) == 0) return TOKEN_KW_FOR;
            if (len == 4 && STRNCASECMP(s, "from", 4) == 0) return TOKEN_KW_FROM;
            break;
        case 'g':
            if (len == 6 && STRNCASECMP(s, "global", 6) == 0) return TOKEN_KW_GLOBAL;
            if (len == 3 && STRNCASECMP(s, "get", 3) == 0) return TOKEN_KW_GET;
            break;
        case 'i':
            if (len == 2 && STRNCASECMP(s, "if", 2) == 0) return TOKEN_KW_IF;
            if (len == 2 && STRNCASECMP(s, "id", 2) == 0) return TOKEN_KW_ID;
            if (len == 2 && STRNCASECMP(s, "in", 2) == 0) return TOKEN_KW_IN;
            if (len == 7 && STRNCASECMP(s, "integer", 7) == 0) return TOKEN_KW_INTEGER;
            if (len == 10 && STRNCASECMP(s, "implements", 10) == 0) return TOKEN_KW_IMPLEMENTS;
            if (len == 9 && STRNCASECMP(s, "inherited", 9) == 0) return TOKEN_KW_INHERITED;
            if (len == 6 && STRNCASECMP(s, "insert", 6) == 0) return TOKEN_KW_DML_INSERT;
            if (len == 10 && STRNCASECMP(s, "instanceof", 10) == 0) return TOKEN_KW_INSTANCEOF;
            if (len == 9 && STRNCASECMP(s, "interface", 9) == 0) return TOKEN_KW_INTERFACE;
            break;
        case 'l':
            if (len == 4 && STRNCASECMP(s, "list", 4) == 0) return TOKEN_KW_LIST;
            if (len == 4 && STRNCASECMP(s, "long", 4) == 0) return TOKEN_KW_LONG;
            if (len == 4 && STRNCASECMP(s, "like", 4) == 0) return TOKEN_KW_LIKE;
            if (len == 5 && STRNCASECMP(s, "limit", 5) == 0) return TOKEN_KW_LIMIT;
            break;
        case 'm':
            if (len == 3 && STRNCASECMP(s, "map", 3) == 0) return TOKEN_KW_MAP;
            if (len == 3 && STRNCASECMP(s, "max", 3) == 0) return TOKEN_KW_MAX;
            if (len == 3 && STRNCASECMP(s, "min", 3) == 0) return TOKEN_KW_MIN;
            if (len == 5 && STRNCASECMP(s, "merge", 5) == 0) return TOKEN_KW_DML_MERGE;
            break;
        case 'n':
            if (len == 3 && STRNCASECMP(s, "new", 3) == 0) return TOKEN_KW_NEW;
            if (len == 4 && STRNCASECMP(s, "null", 4) == 0) return TOKEN_NULL_LITERAL;
            if (len == 3 && STRNCASECMP(s, "not", 3) == 0) return TOKEN_KW_NOT_SOQL;
            break;
        case 'o':
            if (len == 6 && STRNCASECMP(s, "object", 6) == 0) return TOKEN_KW_OBJECT;
            if (len == 8 && STRNCASECMP(s, "override", 8) == 0) return TOKEN_KW_OVERRIDE;
            if (len == 5 && STRNCASECMP(s, "order", 5) == 0) return TOKEN_KW_ORDER;
            if (len == 6 && STRNCASECMP(s, "offset", 6) == 0) return TOKEN_KW_OFFSET;
            if (len == 2 && STRNCASECMP(s, "or", 2) == 0) return TOKEN_KW_OR_SOQL;
            break;
        case 'p':
            if (len == 6 && STRNCASECMP(s, "public", 6) == 0) return TOKEN_KW_PUBLIC;
            if (len == 7 && STRNCASECMP(s, "private", 7) == 0) return TOKEN_KW_PRIVATE;
            if (len == 9 && STRNCASECMP(s, "protected", 9) == 0) return TOKEN_KW_PROTECTED;
            break;
        case 'r':
            if (len == 6 && STRNCASECMP(s, "return", 6) == 0) return TOKEN_KW_RETURN;
            break;
        case 's':
            if (len == 6 && STRNCASECMP(s, "static", 6) == 0) return TOKEN_KW_STATIC;
            if (len == 6 && STRNCASECMP(s, "string", 6) == 0) return TOKEN_KW_STRING;
            if (len == 3 && STRNCASECMP(s, "set", 3) == 0) return TOKEN_KW_SET;
            if (len == 6 && STRNCASECMP(s, "select", 6) == 0) return TOKEN_KW_SELECT;
            if (len == 6 && STRNCASECMP(s, "switch", 6) == 0) return TOKEN_KW_SWITCH;
            if (len == 7 && STRNCASECMP(s, "sharing", 7) == 0) return TOKEN_KW_SHARING;
            if (len == 5 && STRNCASECMP(s, "super", 5) == 0) return TOKEN_KW_SUPER;
            if (len == 6 && STRNCASECMP(s, "system", 6) == 0) return TOKEN_KW_SYSTEM;
            if (len == 3 && STRNCASECMP(s, "sum", 3) == 0) return TOKEN_KW_SUM;
            break;
        case 't':
            if (len == 4 && STRNCASECMP(s, "true", 4) == 0) return TOKEN_BOOL_LITERAL;
            if (len == 4 && STRNCASECMP(s, "this", 4) == 0) return TOKEN_KW_THIS;
            if (len == 5 && STRNCASECMP(s, "throw", 5) == 0) return TOKEN_KW_THROW;
            if (len == 9 && STRNCASECMP(s, "transient", 9) == 0) return TOKEN_KW_TRANSIENT;
            if (len == 7 && STRNCASECMP(s, "trigger", 7) == 0) return TOKEN_KW_TRIGGER;
            if (len == 3 && STRNCASECMP(s, "try", 3) == 0) return TOKEN_KW_TRY;
            if (len == 4 && STRNCASECMP(s, "time", 4) == 0) return TOKEN_KW_TIME;
            if (len == 10 && STRNCASECMP(s, "testmethod", 10) == 0) return TOKEN_KW_TESTMETHOD;
            break;
        case 'u':
            if (len == 6 && STRNCASECMP(s, "update", 6) == 0) return TOKEN_KW_DML_UPDATE;
            if (len == 6 && STRNCASECMP(s, "upsert", 6) == 0) return TOKEN_KW_DML_UPSERT;
            if (len == 8 && STRNCASECMP(s, "undelete", 8) == 0) return TOKEN_KW_DML_UNDELETE;
            break;
        case 'v':
            if (len == 4 && STRNCASECMP(s, "void", 4) == 0) return TOKEN_KW_VOID;
            if (len == 7 && STRNCASECMP(s, "virtual", 7) == 0) return TOKEN_KW_VIRTUAL;
            break;
        case 'w':
            if (len == 5 && STRNCASECMP(s, "while", 5) == 0) return TOKEN_KW_WHILE;
            if (len == 4 && STRNCASECMP(s, "when", 4) == 0) return TOKEN_KW_WHEN;
            if (len == 4 && STRNCASECMP(s, "with", 4) == 0) return TOKEN_KW_WITH;
            if (len == 7 && STRNCASECMP(s, "without", 7) == 0) return TOKEN_KW_WITHOUT;
            if (len == 5 && STRNCASECMP(s, "where", 5) == 0) return TOKEN_KW_WHERE;
            if (len == 10 && STRNCASECMP(s, "webservice", 10) == 0) return TOKEN_KW_WEBSERVICE;
            break;
    }

    return TOKEN_IDENTIFIER;
}

Token lexer_next_token(Lexer* lexer) {
    skip_whitespace_and_comments(lexer);

    lexer->start = lexer->current;

    if (is_at_end(lexer)) return make_token(lexer, TOKEN_EOF);

    char c = advance(lexer);

    // id or keyword
    if (isalpha((unsigned char)c) || c == '_') {
        while (isalnum((unsigned char)peek(lexer)) || peek(lexer) == '_') {
            advance(lexer);
        }
        return make_token(lexer, identifier_type(lexer));
    }

    // numbers
    if (isdigit((unsigned char)c)) {
        bool is_double = false;
        while (isdigit((unsigned char)peek(lexer))) advance(lexer);

        if (peek(lexer) == '.' && isdigit((unsigned char)peek_next(lexer))) {
            is_double = true;
            advance(lexer);
            while (isdigit((unsigned char)peek(lexer))) advance(lexer);
        }

        if (peek(lexer) == 'e' || peek(lexer) == 'E') {
            is_double = true;
            advance(lexer);
            if (peek(lexer) == '+' || peek(lexer) == '-') advance(lexer);
            while (isdigit((unsigned char)peek(lexer))) advance(lexer);
        }

        if (peek(lexer) == 'L' || peek(lexer) == 'l') {
            advance(lexer);
        } else if (peek(lexer) == 'd' || peek(lexer) == 'D' || peek(lexer) == 'm' || peek(lexer) == 'M') {
            is_double = true;
            advance(lexer);
        }

        return make_token(lexer, is_double ? TOKEN_DOUBLE_LITERAL : TOKEN_INT_LITERAL);
    }

    // single quote strings
    if (c == '\'') {
        while (!is_at_end(lexer) && peek(lexer) != '\'') {
            if (peek(lexer) == '\\') {
                advance(lexer);
                if (!is_at_end(lexer)) advance(lexer);
            } else {
                advance(lexer);
            }
        }
        if (is_at_end(lexer)) {
            return error_token(lexer, "Unterminated string literal.");
        }
        advance(lexer);
        return make_token(lexer, TOKEN_STRING_LITERAL);
    }

    // symbols and ops
    switch (c) {
        case '(': return make_token(lexer, TOKEN_LPAREN);
        case ')': return make_token(lexer, TOKEN_RPAREN);
        case '{': return make_token(lexer, TOKEN_LBRACE);
        case '}': return make_token(lexer, TOKEN_RBRACE);
        case '[': return make_token(lexer, TOKEN_LBRACKET);
        case ']': return make_token(lexer, TOKEN_RBRACKET);
        case ';': return make_token(lexer, TOKEN_SEMICOLON);
        case ',': return make_token(lexer, TOKEN_COMMA);
        case '.': return make_token(lexer, TOKEN_DOT);
        case '@': return make_token(lexer, TOKEN_AT);
        case ':': return make_token(lexer, TOKEN_COLON);
        case '%': return make_token(lexer, TOKEN_PERCENT);

        case '+':
            if (match(lexer, '+')) return make_token(lexer, TOKEN_PLUS_PLUS);
            if (match(lexer, '=')) return make_token(lexer, TOKEN_PLUS_ASSIGN);
            return make_token(lexer, TOKEN_PLUS);

        case '-':
            if (match(lexer, '-')) return make_token(lexer, TOKEN_MINUS_MINUS);
            if (match(lexer, '=')) return make_token(lexer, TOKEN_MINUS_ASSIGN);
            return make_token(lexer, TOKEN_MINUS);

        case '*':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_STAR_ASSIGN);
            return make_token(lexer, TOKEN_STAR);

        case '/':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_SLASH_ASSIGN);
            return make_token(lexer, TOKEN_SLASH);

        case '=':
            if (match(lexer, '=')) {
                if (match(lexer, '=')) return make_token(lexer, TOKEN_EXACT_EQUAL);
                return make_token(lexer, TOKEN_EQUAL);
            }
            if (match(lexer, '>')) return make_token(lexer, TOKEN_ARROW);
            return make_token(lexer, TOKEN_ASSIGN);

        case '!':
            if (match(lexer, '=')) {
                if (match(lexer, '=')) return make_token(lexer, TOKEN_EXACT_NOT_EQUAL);
                return make_token(lexer, TOKEN_NOT_EQUAL);
            }
            return make_token(lexer, TOKEN_NOT);

        case '<':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_LE);
            if (match(lexer, '>')) return make_token(lexer, TOKEN_NOT_EQUAL);
            return make_token(lexer, TOKEN_LT);

        case '>':
            if (match(lexer, '=')) return make_token(lexer, TOKEN_GE);
            return make_token(lexer, TOKEN_GT);

        case '&':
            if (match(lexer, '&')) return make_token(lexer, TOKEN_AND);
            return error_token(lexer, "Unexpected character '&'");

        case '|':
            if (match(lexer, '|')) return make_token(lexer, TOKEN_OR);
            return error_token(lexer, "Unexpected character '|'");

        case '?':
            if (match(lexer, '.')) return make_token(lexer, TOKEN_SAFE_DOT);
            if (match(lexer, '?')) return make_token(lexer, TOKEN_NULL_COALESCE);
            return make_token(lexer, TOKEN_QUESTION);
    }

    return error_token(lexer, "Unexpected character.");
}

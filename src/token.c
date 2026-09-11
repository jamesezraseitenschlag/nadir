// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#include "token.h"

const char* token_type_name(TokenType type) {
    switch (type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_INT_LITERAL: return "INT_LITERAL";
        case TOKEN_DOUBLE_LITERAL: return "DOUBLE_LITERAL";
        case TOKEN_STRING_LITERAL: return "STRING_LITERAL";
        case TOKEN_BOOL_LITERAL: return "BOOL_LITERAL";
        case TOKEN_NULL_LITERAL: return "NULL_LITERAL";
        case TOKEN_KW_CLASS: return "CLASS";
        case TOKEN_KW_PUBLIC: return "PUBLIC";
        case TOKEN_KW_PRIVATE: return "PRIVATE";
        case TOKEN_KW_STATIC: return "STATIC";
        case TOKEN_KW_VOID: return "VOID";
        case TOKEN_KW_INTEGER: return "INTEGER";
        case TOKEN_KW_STRING: return "STRING";
        case TOKEN_KW_BOOLEAN: return "BOOLEAN";
        case TOKEN_KW_IF: return "IF";
        case TOKEN_KW_ELSE: return "ELSE";
        case TOKEN_KW_WHILE: return "WHILE";
        case TOKEN_KW_FOR: return "FOR";
        case TOKEN_KW_RETURN: return "RETURN";
        case TOKEN_KW_NEW: return "NEW";
        case TOKEN_KW_DML_INSERT: return "INSERT";
        case TOKEN_KW_DML_UPDATE: return "UPDATE";
        case TOKEN_KW_DML_DELETE: return "DELETE";
        case TOKEN_KW_SELECT: return "SELECT";
        case TOKEN_KW_FROM: return "FROM";
        case TOKEN_KW_WHERE: return "WHERE";
        case TOKEN_PLUS: return "+";
        case TOKEN_MINUS: return "-";
        case TOKEN_STAR: return "*";
        case TOKEN_SLASH: return "/";
        case TOKEN_ASSIGN: return "=";
        case TOKEN_EQUAL: return "==";
        case TOKEN_NOT_EQUAL: return "!=";
        case TOKEN_SAFE_DOT: return "?.";
        case TOKEN_DOT: return ".";
        case TOKEN_SEMICOLON: return ";";
        case TOKEN_COMMA: return ",";
        case TOKEN_LPAREN: return "(";
        case TOKEN_RPAREN: return ")";
        case TOKEN_LBRACE: return "{";
        case TOKEN_RBRACE: return "}";
        case TOKEN_LBRACKET: return "[";
        case TOKEN_RBRACKET: return "]";
        default: return "TOKEN";
    }
}

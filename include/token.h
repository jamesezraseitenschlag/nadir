// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#ifndef NADIR_TOKEN_H
#define NADIR_TOKEN_H

typedef enum {
    TOKEN_EOF = 0,
    TOKEN_ERROR,

    // Literals & Identifiers
    TOKEN_IDENTIFIER,
    TOKEN_INT_LITERAL,
    TOKEN_DOUBLE_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_BOOL_LITERAL,
    TOKEN_NULL_LITERAL,

    // Apex Class & OOP Keywords
    TOKEN_KW_CLASS,
    TOKEN_KW_INTERFACE,
    TOKEN_KW_EXTENDS,
    TOKEN_KW_IMPLEMENTS,
    TOKEN_KW_ENUM,
    TOKEN_KW_PUBLIC,
    TOKEN_KW_PRIVATE,
    TOKEN_KW_PROTECTED,
    TOKEN_KW_GLOBAL,
    TOKEN_KW_STATIC,
    TOKEN_KW_FINAL,
    TOKEN_KW_ABSTRACT,
    TOKEN_KW_VIRTUAL,
    TOKEN_KW_OVERRIDE,
    TOKEN_KW_TRANSIENT,
    TOKEN_KW_TESTMETHOD,
    TOKEN_KW_WEBSERVICE,
    TOKEN_KW_WITH,
    TOKEN_KW_WITHOUT,
    TOKEN_KW_INHERITED,
    TOKEN_KW_SHARING,

    // Types
    TOKEN_KW_VOID,
    TOKEN_KW_INTEGER,
    TOKEN_KW_DOUBLE,
    TOKEN_KW_DECIMAL,
    TOKEN_KW_LONG,
    TOKEN_KW_STRING,
    TOKEN_KW_BOOLEAN,
    TOKEN_KW_DATE,
    TOKEN_KW_DATETIME,
    TOKEN_KW_TIME,
    TOKEN_KW_BLOB,
    TOKEN_KW_ID,
    TOKEN_KW_OBJECT,
    TOKEN_KW_LIST,
    TOKEN_KW_SET,
    TOKEN_KW_MAP,

    // Control Flow
    TOKEN_KW_IF,
    TOKEN_KW_ELSE,
    TOKEN_KW_SWITCH,
    TOKEN_KW_WHEN,
    TOKEN_KW_WHILE,
    TOKEN_KW_DO,
    TOKEN_KW_FOR,
    TOKEN_KW_BREAK,
    TOKEN_KW_CONTINUE,
    TOKEN_KW_RETURN,
    TOKEN_KW_TRY,
    TOKEN_KW_CATCH,
    TOKEN_KW_FINALLY,
    TOKEN_KW_THROW,

    // OOP & Context
    TOKEN_KW_NEW,
    TOKEN_KW_THIS,
    TOKEN_KW_SUPER,
    TOKEN_KW_INSTANCEOF,
    TOKEN_KW_GET,
    TOKEN_KW_SET_PROP,
    TOKEN_KW_TRIGGER,
    TOKEN_KW_BEFORE,
    TOKEN_KW_AFTER,
    TOKEN_KW_SYSTEM,

    // DML
    TOKEN_KW_DML_INSERT,
    TOKEN_KW_DML_UPDATE,
    TOKEN_KW_DML_UPSERT,
    TOKEN_KW_DML_DELETE,
    TOKEN_KW_DML_UNDELETE,
    TOKEN_KW_DML_MERGE,

    // SOQL
    TOKEN_KW_SELECT,
    TOKEN_KW_FROM,
    TOKEN_KW_WHERE,
    TOKEN_KW_ORDER,
    TOKEN_KW_BY,
    TOKEN_KW_LIMIT,
    TOKEN_KW_OFFSET,
    TOKEN_KW_ASC,
    TOKEN_KW_DESC,
    TOKEN_KW_LIKE,
    TOKEN_KW_IN,
    TOKEN_KW_AND_SOQL,
    TOKEN_KW_OR_SOQL,
    TOKEN_KW_NOT_SOQL,
    TOKEN_KW_COUNT,
    TOKEN_KW_SUM,
    TOKEN_KW_AVG,
    TOKEN_KW_MIN,
    TOKEN_KW_MAX,

    // Operators & Symbols
    TOKEN_PLUS,          // +
    TOKEN_MINUS,         // -
    TOKEN_STAR,          // *
    TOKEN_SLASH,         // /
    TOKEN_PERCENT,       // %
    TOKEN_PLUS_PLUS,     // ++
    TOKEN_MINUS_MINUS,   // --

    TOKEN_ASSIGN,        // =
    TOKEN_PLUS_ASSIGN,   // +=
    TOKEN_MINUS_ASSIGN,  // -=
    TOKEN_STAR_ASSIGN,   // *=
    TOKEN_SLASH_ASSIGN,  // /=

    TOKEN_EQUAL,         // ==
    TOKEN_NOT_EQUAL,     // != or <>
    TOKEN_EXACT_EQUAL,   // ===
    TOKEN_EXACT_NOT_EQUAL,// !==
    TOKEN_LT,            // <
    TOKEN_GT,            // >
    TOKEN_LE,            // <=
    TOKEN_GE,            // >=

    TOKEN_AND,           // &&
    TOKEN_OR,            // ||
    TOKEN_NOT,           // !

    TOKEN_QUESTION,      // ?
    TOKEN_COLON,         // :
    TOKEN_SAFE_DOT,      // ?.
    TOKEN_NULL_COALESCE, // ??
    TOKEN_ARROW,         // =>
    TOKEN_AT,            // @

    TOKEN_DOT,           // .
    TOKEN_COMMA,         // ,
    TOKEN_SEMICOLON,     // ;
    TOKEN_LPAREN,        // (
    TOKEN_RPAREN,        // )
    TOKEN_LBRACE,        // {
    TOKEN_RBRACE,        // }
    TOKEN_LBRACKET,      // [
    TOKEN_RBRACKET       // ]
} TokenType;

typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
    int column;
} Token;

const char* token_type_name(TokenType type);

#endif // TOKEN_H

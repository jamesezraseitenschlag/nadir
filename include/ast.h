// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#ifndef NADIR_AST_H
#define NADIR_AST_H

#include "common.h"
#include "value.h"

typedef enum {
    NODE_PROGRAM,
    NODE_LITERAL,
    NODE_IDENTIFIER,
    NODE_VAR_DECL,
    NODE_ASSIGN,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_TERNARY,
    NODE_BLOCK,
    NODE_IF,
    NODE_WHILE,
    NODE_DO_WHILE,
    NODE_FOR,
    NODE_FOR_EACH,
    NODE_SWITCH,
    NODE_TRY_CATCH,
    NODE_RETURN,
    NODE_CALL,
    NODE_MEMBER_ACCESS,
    NODE_NEW,
    NODE_CLASS_DECL,
    NODE_METHOD_DECL,
    NODE_TRIGGER,
    NODE_DML,
    NODE_SOQL
} ASTNodeType;

struct ASTNode;

typedef struct ASTNodeArray {
    struct ASTNode** nodes;
    int count;
    int capacity;
} ASTNodeArray;

typedef struct ParamDecl {
    char* type_name;
    char* param_name;
} ParamDecl;

typedef struct SwitchCase {
    struct ASTNode* match_val; // NULL for 'when else'
    struct ASTNode* body;
} SwitchCase;

typedef struct CatchClause {
    char* exception_type;
    char* var_name;
    struct ASTNode* body;
} CatchClause;

typedef struct ASTNode {
    ASTNodeType type;
    int line;
    union {
        struct {
            ASTNodeArray statements;
        } program;

        struct {
            Value val;
        } literal;

        struct {
            char* name;
        } identifier;

        struct {
            char* type_name;
            char* var_name;
            struct ASTNode* init;
        } var_decl;

        struct {
            struct ASTNode* target;
            char* op;
            struct ASTNode* value;
        } assign;

        struct {
            struct ASTNode* left;
            char* op;
            struct ASTNode* right;
        } binary;

        struct {
            char* op;
            struct ASTNode* operand;
            bool prefix;
        } unary;

        struct {
            struct ASTNode* cond;
            struct ASTNode* then_expr;
            struct ASTNode* else_expr;
        } ternary;

        struct {
            ASTNodeArray statements;
        } block;

        struct {
            struct ASTNode* cond;
            struct ASTNode* then_b;
            struct ASTNode* else_b;
        } if_stmt;

        struct {
            struct ASTNode* cond;
            struct ASTNode* body;
        } while_stmt;

        struct {
            struct ASTNode* cond;
            struct ASTNode* body;
        } do_while;

        struct {
            struct ASTNode* init;
            struct ASTNode* cond;
            struct ASTNode* update;
            struct ASTNode* body;
        } for_stmt;

        struct {
            char* item_type;
            char* item_name;
            struct ASTNode* collection;
            struct ASTNode* body;
        } for_each;

        struct {
            struct ASTNode* target;
            SwitchCase* cases;
            int case_count;
        } switch_stmt;

        struct {
            struct ASTNode* try_block;
            CatchClause* catches;
            int catch_count;
            struct ASTNode* finally_block;
        } try_catch;

        struct {
            struct ASTNode* value;
        } return_stmt;

        struct {
            struct ASTNode* callee;
            char* method_name;
            ASTNodeArray args;
            bool safe_nav;
        } call;

        struct {
            struct ASTNode* target;
            char* member_name;
            bool safe_nav;
        } member_access;

        struct {
            char* type_name;
            ASTNodeArray args;
            ASTNodeArray list_init;
            ASTNodeArray map_keys;
            ASTNodeArray map_vals;
        } new_expr;

        struct {
            char* name;
            char* parent_class;
            ASTNodeArray members;
        } class_decl;

        struct {
            char* name;
            char* return_type;
            ParamDecl* params;
            int param_count;
            struct ASTNode* body;
            bool is_static;
        } method_decl;

        struct {
            char* name;
            char* sobject_name;
            char* events;
            struct ASTNode* body;
        } trigger;

        struct {
            char* operation; // "insert", "update", "delete", "upsert", "undelete", "merge"
            struct ASTNode* target;
        } dml;

        struct {
            char* from_object;
            char** fields;
            int field_count;
            char* where_field;
            char* where_op;
            struct ASTNode* where_val;
            int limit;
        } soql;
    } as;
} ASTNode;

ASTNode* ast_new_node(ASTNodeType type, int line);
void ast_node_free(ASTNode* node);

#endif // AST_H

// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#include "ast.h"

ASTNode* ast_new_node(ASTNodeType type, int line) {
    ASTNode* n = (ASTNode*)malloc(sizeof(ASTNode));
    memset(n, 0, sizeof(ASTNode));
    n->type = type;
    n->line = line;
    return n;
}

void ast_node_array_append(ASTNodeArray* arr, ASTNode* node) {
    if (arr->count + 1 > arr->capacity) {
        arr->capacity = arr->capacity < 8 ? 8 : arr->capacity * 2;
        arr->nodes = (ASTNode**)realloc(arr->nodes, sizeof(ASTNode*) * arr->capacity);
    }
    arr->nodes[arr->count++] = node;
}

void ast_node_free(ASTNode* n) {
    if (!n) return;
    switch (n->type) {
        case NODE_PROGRAM:
            for (int i = 0; i < n->as.program.statements.count; i++) {
                ast_node_free(n->as.program.statements.nodes[i]);
            }
            if (n->as.program.statements.nodes) free(n->as.program.statements.nodes);
            break;
        case NODE_BLOCK:
            for (int k = 0; k < n->as.block.statements.count; k++) {
                ast_node_free(n->as.block.statements.nodes[k]);
            }
            if (n->as.block.statements.nodes) free(n->as.block.statements.nodes);
            break;
        case NODE_IDENTIFIER:
            if (n->as.identifier.name) free(n->as.identifier.name);
            break;
        case NODE_VAR_DECL:
            if (n->as.var_decl.type_name) free(n->as.var_decl.type_name);
            if (n->as.var_decl.var_name) free(n->as.var_decl.var_name);
            ast_node_free(n->as.var_decl.init);
            break;
        case NODE_ASSIGN:
            ast_node_free(n->as.assign.target);
            if (n->as.assign.op) free(n->as.assign.op);
            ast_node_free(n->as.assign.value);
            break;
        case NODE_BINARY_OP:
            ast_node_free(n->as.binary.left);
            if (n->as.binary.op) free(n->as.binary.op);
            ast_node_free(n->as.binary.right);
            break;
        case NODE_UNARY_OP:
            if (n->as.unary.op) free(n->as.unary.op);
            ast_node_free(n->as.unary.operand);
            break;
        case NODE_TERNARY:
            ast_node_free(n->as.ternary.cond);
            ast_node_free(n->as.ternary.then_expr);
            ast_node_free(n->as.ternary.else_expr);
            break;
        case NODE_IF:
            ast_node_free(n->as.if_stmt.cond);
            ast_node_free(n->as.if_stmt.then_b);
            ast_node_free(n->as.if_stmt.else_b);
            break;
        case NODE_WHILE:
            ast_node_free(n->as.while_stmt.cond);
            ast_node_free(n->as.while_stmt.body);
            break;
        case NODE_DO_WHILE:
            ast_node_free(n->as.do_while.body);
            ast_node_free(n->as.do_while.cond);
            break;
        case NODE_FOR:
            ast_node_free(n->as.for_stmt.init);
            ast_node_free(n->as.for_stmt.cond);
            ast_node_free(n->as.for_stmt.update);
            ast_node_free(n->as.for_stmt.body);
            break;
        case NODE_FOR_EACH:
            if (n->as.for_each.item_type) free(n->as.for_each.item_type);
            if (n->as.for_each.item_name) free(n->as.for_each.item_name);
            ast_node_free(n->as.for_each.collection);
            ast_node_free(n->as.for_each.body);
            break;
        case NODE_SWITCH:
            ast_node_free(n->as.switch_stmt.target);
            for (int i = 0; i < n->as.switch_stmt.case_count; i++) {
                ast_node_free(n->as.switch_stmt.cases[i].match_val);
                ast_node_free(n->as.switch_stmt.cases[i].body);
            }
            if (n->as.switch_stmt.cases) free(n->as.switch_stmt.cases);
            break;
        case NODE_TRY_CATCH:
            ast_node_free(n->as.try_catch.try_block);
            for (int i = 0; i < n->as.try_catch.catch_count; i++) {
                if (n->as.try_catch.catches[i].exception_type) free(n->as.try_catch.catches[i].exception_type);
                if (n->as.try_catch.catches[i].var_name) free(n->as.try_catch.catches[i].var_name);
                ast_node_free(n->as.try_catch.catches[i].body);
            }
            if (n->as.try_catch.catches) free(n->as.try_catch.catches);
            ast_node_free(n->as.try_catch.finally_block);
            break;
        case NODE_RETURN:
            ast_node_free(n->as.return_stmt.value);
            break;
        case NODE_CALL:
            ast_node_free(n->as.call.callee);
            if (n->as.call.method_name) free(n->as.call.method_name);
            for (int i = 0; i < n->as.call.args.count; i++) {
                ast_node_free(n->as.call.args.nodes[i]);
            }
            if (n->as.call.args.nodes) free(n->as.call.args.nodes);
            break;
        case NODE_MEMBER_ACCESS:
            ast_node_free(n->as.member_access.target);
            if (n->as.member_access.member_name) free(n->as.member_access.member_name);
            break;
        case NODE_NEW:
            if (n->as.new_expr.type_name) free(n->as.new_expr.type_name);
            for (int i = 0; i < n->as.new_expr.args.count; i++) {
                ast_node_free(n->as.new_expr.args.nodes[i]);
            }
            if (n->as.new_expr.args.nodes) free(n->as.new_expr.args.nodes);
            for (int i = 0; i < n->as.new_expr.list_init.count; i++) {
                ast_node_free(n->as.new_expr.list_init.nodes[i]);
            }
            if (n->as.new_expr.list_init.nodes) free(n->as.new_expr.list_init.nodes);
            break;
        case NODE_CLASS_DECL:
            if (n->as.class_decl.name) free(n->as.class_decl.name);
            if (n->as.class_decl.parent_class) free(n->as.class_decl.parent_class);
            for (int i = 0; i < n->as.class_decl.members.count; i++) {
                ast_node_free(n->as.class_decl.members.nodes[i]);
            }
            if (n->as.class_decl.members.nodes) free(n->as.class_decl.members.nodes);
            break;
        case NODE_METHOD_DECL:
            if (n->as.method_decl.name) free(n->as.method_decl.name);
            if (n->as.method_decl.return_type) free(n->as.method_decl.return_type);
            for (int p = 0; p < n->as.method_decl.param_count; p++) {
                if (n->as.method_decl.params[p].type_name) free(n->as.method_decl.params[p].type_name);
                if (n->as.method_decl.params[p].param_name) free(n->as.method_decl.params[p].param_name);
            }
            if (n->as.method_decl.params) free(n->as.method_decl.params);
            ast_node_free(n->as.method_decl.body);
            break;
        case NODE_TRIGGER:
            if (n->as.trigger.name) free(n->as.trigger.name);
            if (n->as.trigger.sobject_name) free(n->as.trigger.sobject_name);
            if (n->as.trigger.events) free(n->as.trigger.events);
            ast_node_free(n->as.trigger.body);
            break;
        case NODE_DML:
            if (n->as.dml.operation) free(n->as.dml.operation);
            ast_node_free(n->as.dml.target);
            break;
        case NODE_SOQL:
            if (n->as.soql.from_object) free(n->as.soql.from_object);
            for (int f = 0; f < n->as.soql.field_count; f++) {
                if (n->as.soql.fields[f]) free(n->as.soql.fields[f]);
            }
            if (n->as.soql.fields) free(n->as.soql.fields);
            if (n->as.soql.where_field) free(n->as.soql.where_field);
            if (n->as.soql.where_op) free(n->as.soql.where_op);
            ast_node_free(n->as.soql.where_val);
            break;
        default:
            break;
    }
    free(n);
}


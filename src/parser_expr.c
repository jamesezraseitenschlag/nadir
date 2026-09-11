// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#include "parser_internal.h"

ASTNode* parse_expression(Parser* parser) {
    return parse_assignment(parser);
}

ASTNode* parse_assignment(Parser* parser) {
    ASTNode* expr = parse_ternary(parser);

    if (match_token(parser, TOKEN_ASSIGN) || match_token(parser, TOKEN_PLUS_ASSIGN) ||
        match_token(parser, TOKEN_MINUS_ASSIGN) || match_token(parser, TOKEN_STAR_ASSIGN) ||
        match_token(parser, TOKEN_SLASH_ASSIGN)) {
        char* op = duplicate_slice(parser->previous.start, parser->previous.length);
        ASTNode* value = parse_assignment(parser);

        ASTNode* assign_node = ast_new_node(NODE_ASSIGN, parser->previous.line);
        assign_node->as.assign.target = expr;
        assign_node->as.assign.op = op;
        assign_node->as.assign.value = value;
        return assign_node;
    }

    return expr;
}

ASTNode* parse_ternary(Parser* parser) {
    ASTNode* expr = parse_logical_or(parser);
    if (match_token(parser, TOKEN_QUESTION)) {
        int line = parser->previous.line;
        ASTNode* then_expr = parse_expression(parser);
        consume_token(parser, TOKEN_COLON, "Expected ':' in ternary expression.");
        ASTNode* else_expr = parse_expression(parser);
        ASTNode* tern = ast_new_node(NODE_TERNARY, line);
        tern->as.ternary.cond = expr;
        tern->as.ternary.then_expr = then_expr;
        tern->as.ternary.else_expr = else_expr;
        return tern;
    }
    return expr;
}

ASTNode* parse_logical_or(Parser* parser) {
    ASTNode* expr = parse_null_coalescing(parser);
    while (match_token(parser, TOKEN_OR)) {
        char* op = duplicate_slice(parser->previous.start, parser->previous.length);
        ASTNode* right = parse_null_coalescing(parser);
        ASTNode* bin = ast_new_node(NODE_BINARY_OP, parser->previous.line);
        bin->as.binary.left = expr;
        bin->as.binary.op = op;
        bin->as.binary.right = right;
        expr = bin;
    }
    return expr;
}

ASTNode* parse_null_coalescing(Parser* parser) {
    ASTNode* expr = parse_logical_and(parser);
    while (match_token(parser, TOKEN_NULL_COALESCE)) {
        char* op = duplicate_string("??");
        ASTNode* right = parse_logical_and(parser);
        ASTNode* bin = ast_new_node(NODE_BINARY_OP, parser->previous.line);
        bin->as.binary.left = expr;
        bin->as.binary.op = op;
        bin->as.binary.right = right;
        expr = bin;
    }
    return expr;
}

ASTNode* parse_logical_and(Parser* parser) {
    ASTNode* expr = parse_equality(parser);
    while (match_token(parser, TOKEN_AND)) {
        char* op = duplicate_slice(parser->previous.start, parser->previous.length);
        ASTNode* right = parse_equality(parser);
        ASTNode* bin = ast_new_node(NODE_BINARY_OP, parser->previous.line);
        bin->as.binary.left = expr;
        bin->as.binary.op = op;
        bin->as.binary.right = right;
        expr = bin;
    }
    return expr;
}

ASTNode* parse_equality(Parser* parser) {
    ASTNode* expr = parse_relational(parser);
    while (match_token(parser, TOKEN_EQUAL) || match_token(parser, TOKEN_NOT_EQUAL) ||
           match_token(parser, TOKEN_EXACT_EQUAL) || match_token(parser, TOKEN_EXACT_NOT_EQUAL)) {
        char* op = duplicate_slice(parser->previous.start, parser->previous.length);
        ASTNode* right = parse_relational(parser);
        ASTNode* bin = ast_new_node(NODE_BINARY_OP, parser->previous.line);
        bin->as.binary.left = expr;
        bin->as.binary.op = op;
        bin->as.binary.right = right;
        expr = bin;
    }
    return expr;
}

ASTNode* parse_relational(Parser* parser) {
    ASTNode* expr = parse_additive(parser);
    while (match_token(parser, TOKEN_LT) || match_token(parser, TOKEN_GT) ||
           match_token(parser, TOKEN_LE) || match_token(parser, TOKEN_GE)) {
        char* op = duplicate_slice(parser->previous.start, parser->previous.length);
        ASTNode* right = parse_additive(parser);
        ASTNode* bin = ast_new_node(NODE_BINARY_OP, parser->previous.line);
        bin->as.binary.left = expr;
        bin->as.binary.op = op;
        bin->as.binary.right = right;
        expr = bin;
    }
    return expr;
}

ASTNode* parse_additive(Parser* parser) {
    ASTNode* expr = parse_multiplicative(parser);
    while (match_token(parser, TOKEN_PLUS) || match_token(parser, TOKEN_MINUS)) {
        char* op = duplicate_slice(parser->previous.start, parser->previous.length);
        ASTNode* right = parse_multiplicative(parser);
        ASTNode* bin = ast_new_node(NODE_BINARY_OP, parser->previous.line);
        bin->as.binary.left = expr;
        bin->as.binary.op = op;
        bin->as.binary.right = right;
        expr = bin;
    }
    return expr;
}

ASTNode* parse_multiplicative(Parser* parser) {
    ASTNode* expr = parse_unary(parser);
    while (match_token(parser, TOKEN_STAR) || match_token(parser, TOKEN_SLASH) || match_token(parser, TOKEN_PERCENT)) {
        char* op = duplicate_slice(parser->previous.start, parser->previous.length);
        ASTNode* right = parse_unary(parser);
        ASTNode* bin = ast_new_node(NODE_BINARY_OP, parser->previous.line);
        bin->as.binary.left = expr;
        bin->as.binary.op = op;
        bin->as.binary.right = right;
        expr = bin;
    }
    return expr;
}

ASTNode* parse_unary(Parser* parser) {
    if (match_token(parser, TOKEN_NOT) || match_token(parser, TOKEN_MINUS) || match_token(parser, TOKEN_PLUS)) {
        char* op = duplicate_slice(parser->previous.start, parser->previous.length);
        ASTNode* operand = parse_unary(parser);
        ASTNode* un = ast_new_node(NODE_UNARY_OP, parser->previous.line);
        un->as.unary.op = op;
        un->as.unary.operand = operand;
        un->as.unary.prefix = true;
        return un;
    }

    if (check_token(parser, TOKEN_LPAREN)) {
        const char* saved_start = parser->lexer->start;
        const char* saved_cur = parser->lexer->current;
        int saved_line = parser->lexer->line;
        int saved_col = parser->lexer->column;
        Token saved_current = parser->current;
        Token saved_prev = parser->previous;

        advance_token(parser);
        if (is_type_token(parser->current.type) || is_identifier_token(parser->current.type)) {
            char* cast_type = parse_type_name(parser);
            if (match_token(parser, TOKEN_RPAREN)) {
                if (check_token(parser, TOKEN_IDENTIFIER) || check_token(parser, TOKEN_LPAREN) || check_token(parser, TOKEN_KW_THIS) ||
                    check_token(parser, TOKEN_KW_SUPER) || check_token(parser, TOKEN_KW_NEW) || check_token(parser, TOKEN_INT_LITERAL) ||
                    check_token(parser, TOKEN_DOUBLE_LITERAL) || check_token(parser, TOKEN_STRING_LITERAL) || check_token(parser, TOKEN_LBRACKET) ||
                    check_token(parser, TOKEN_BOOL_LITERAL) || check_token(parser, TOKEN_NULL_LITERAL)) {
                    free(cast_type);
                    return parse_unary(parser);
                }
            }
            free(cast_type);
        }

        parser->lexer->start = saved_start;
        parser->lexer->current = saved_cur;
        parser->lexer->line = saved_line;
        parser->lexer->column = saved_col;
        parser->current = saved_current;
        parser->previous = saved_prev;
    }

    return parse_postfix(parser);
}

ASTNode* parse_postfix(Parser* parser) {
    ASTNode* expr = parse_primary(parser);

    while (true) {
        if (match_token(parser, TOKEN_DOT) || match_token(parser, TOKEN_SAFE_DOT)) {
            bool safe_nav = (parser->previous.type == TOKEN_SAFE_DOT);
            char* name = consume_identifier_name(parser, "Expected property or method name.");

            if (match_token(parser, TOKEN_LPAREN)) {
                ASTNode* call_node = ast_new_node(NODE_CALL, parser->previous.line);
                call_node->as.call.callee = expr;
                call_node->as.call.method_name = name;
                call_node->as.call.safe_nav = safe_nav;
                if (!check_token(parser, TOKEN_RPAREN)) {
                    do {
                        ast_node_array_append(&call_node->as.call.args, parse_expression(parser));
                    } while (match_token(parser, TOKEN_COMMA));
                }
                consume_token(parser, TOKEN_RPAREN, "Expected ')' after method arguments.");
                expr = call_node;
            } else {
                ASTNode* mem = ast_new_node(NODE_MEMBER_ACCESS, parser->previous.line);
                mem->as.member_access.target = expr;
                mem->as.member_access.member_name = name;
                mem->as.member_access.safe_nav = safe_nav;
                expr = mem;
            }
        } else if (match_token(parser, TOKEN_LBRACKET)) {
            ASTNode* index_expr = parse_expression(parser);
            consume_token(parser, TOKEN_RBRACKET, "Expected ']' after index.");
            ASTNode* call_node = ast_new_node(NODE_CALL, parser->previous.line);
            call_node->as.call.callee = expr;
            call_node->as.call.method_name = duplicate_string("get");
            call_node->as.call.safe_nav = false;
            ast_node_array_append(&call_node->as.call.args, index_expr);
            expr = call_node;
        } else if (match_token(parser, TOKEN_PLUS_PLUS)) {
            ASTNode* un = ast_new_node(NODE_UNARY_OP, parser->previous.line);
            un->as.unary.op = duplicate_string("++");
            un->as.unary.operand = expr;
            un->as.unary.prefix = false;
            expr = un;
        } else if (match_token(parser, TOKEN_MINUS_MINUS)) {
            ASTNode* un = ast_new_node(NODE_UNARY_OP, parser->previous.line);
            un->as.unary.op = duplicate_string("--");
            un->as.unary.operand = expr;
            un->as.unary.prefix = false;
            expr = un;
        } else {
            break;
        }
    }

    return expr;
}

ASTNode* parse_primary(Parser* parser) {
    int line = parser->current.line;

    if (match_token(parser, TOKEN_INT_LITERAL)) {
        int64_t val = atoll(parser->previous.start);
        ASTNode* node = ast_new_node(NODE_LITERAL, line);
        node->as.literal.val = val_int(val);
        return node;
    }

    if (match_token(parser, TOKEN_DOUBLE_LITERAL)) {
        double val = atof(parser->previous.start);
        ASTNode* node = ast_new_node(NODE_LITERAL, line);
        node->as.literal.val = val_double(val);
        return node;
    }

    if (match_token(parser, TOKEN_STRING_LITERAL)) {
        const char* s = parser->previous.start + 1;
        int len = parser->previous.length - 2;
        ASTNode* node = ast_new_node(NODE_LITERAL, line);
        node->as.literal.val = val_string_slice(s, len);
        return node;
    }

    if (match_token(parser, TOKEN_BOOL_LITERAL)) {
        bool val = (parser->previous.length == 4 && STRNCASECMP(parser->previous.start, "true", 4) == 0);
        ASTNode* node = ast_new_node(NODE_LITERAL, line);
        node->as.literal.val = val_bool(val);
        return node;
    }

    if (match_token(parser, TOKEN_NULL_LITERAL)) {
        ASTNode* node = ast_new_node(NODE_LITERAL, line);
        node->as.literal.val = val_null();
        return node;
    }

    if (match_token(parser, TOKEN_LBRACKET)) {
        if (check_token(parser, TOKEN_KW_SELECT)) {
            return parse_soql_query(parser);
        }
        int bracket_depth = 1;
        while (bracket_depth > 0 && !check_token(parser, TOKEN_EOF)) {
            if (match_token(parser, TOKEN_LBRACKET)) bracket_depth++;
            else if (match_token(parser, TOKEN_RBRACKET)) bracket_depth--;
            else advance_token(parser);
        }
        ASTNode* node = ast_new_node(NODE_LITERAL, line);
        node->as.literal.val = val_list();
        return node;
    }

    if (match_token(parser, TOKEN_KW_NEW)) {
        char* type_name = parse_type_name(parser);
        ASTNode* new_node = ast_new_node(NODE_NEW, line);
        new_node->as.new_expr.type_name = type_name;

        if (match_token(parser, TOKEN_LPAREN)) {
            if (!check_token(parser, TOKEN_RPAREN)) {
                do {
                    ast_node_array_append(&new_node->as.new_expr.args, parse_expression(parser));
                } while (match_token(parser, TOKEN_COMMA));
            }
            consume_token(parser, TOKEN_RPAREN, "Expected ')' after constructor arguments.");
        } else if (match_token(parser, TOKEN_LBRACE)) {
            if (!check_token(parser, TOKEN_RBRACE)) {
                ASTNode* first_expr = parse_expression(parser);
                if (match_token(parser, TOKEN_ARROW)) {
                    ASTNode* first_val = parse_expression(parser);
                    ast_node_array_append(&new_node->as.new_expr.map_keys, first_expr);
                    ast_node_array_append(&new_node->as.new_expr.map_vals, first_val);
                    while (match_token(parser, TOKEN_COMMA)) {
                        ASTNode* k = parse_expression(parser);
                        consume_token(parser, TOKEN_ARROW, "Expected '=>' in map initializer.");
                        ASTNode* v = parse_expression(parser);
                        ast_node_array_append(&new_node->as.new_expr.map_keys, k);
                        ast_node_array_append(&new_node->as.new_expr.map_vals, v);
                    }
                } else {
                    ast_node_array_append(&new_node->as.new_expr.list_init, first_expr);
                    while (match_token(parser, TOKEN_COMMA)) {
                        ast_node_array_append(&new_node->as.new_expr.list_init, parse_expression(parser));
                    }
                }
            }
            consume_token(parser, TOKEN_RBRACE, "Expected '}' after collection initializer.");
        }
        return new_node;
    }

    if (is_identifier_token(parser->current.type) || check_token(parser, TOKEN_KW_THIS) || check_token(parser, TOKEN_KW_SUPER)) {
        advance_token(parser);
        char* name = duplicate_slice(parser->previous.start, parser->previous.length);
        if (match_token(parser, TOKEN_LPAREN)) {
            ASTNode* call_node = ast_new_node(NODE_CALL, line);
            call_node->as.call.callee = NULL;
            call_node->as.call.method_name = name;
            call_node->as.call.safe_nav = false;
            if (!check_token(parser, TOKEN_RPAREN)) {
                do {
                    ast_node_array_append(&call_node->as.call.args, parse_expression(parser));
                } while (match_token(parser, TOKEN_COMMA));
            }
            consume_token(parser, TOKEN_RPAREN, "Expected ')' after function arguments.");
            return call_node;
        }

        ASTNode* id_node = ast_new_node(NODE_IDENTIFIER, line);
        id_node->as.identifier.name = name;
        return id_node;
    }

    if (match_token(parser, TOKEN_LPAREN)) {
        ASTNode* expr = parse_expression(parser);
        consume_token(parser, TOKEN_RPAREN, "Expected ')' after expression.");
        return expr;
    }

    fprintf(stderr, "[Line %d] Parser Error: Unexpected token '%.*s'\n",
            parser->current.line, parser->current.length, parser->current.start);
    parser->had_error = true;
    advance_token(parser);
    return NULL;
}

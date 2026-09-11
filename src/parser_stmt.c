// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#include "parser_internal.h"

ASTNode* parse_block(Parser* parser) {
    int line = parser->previous.line;
    consume_token(parser, TOKEN_LBRACE, "Expected '{' at start of block.");
    ASTNode* block = ast_new_node(NODE_BLOCK, line);
    while (!check_token(parser, TOKEN_RBRACE) && !check_token(parser, TOKEN_EOF)) {
        ASTNode* stmt = parse_declaration(parser);
        if (stmt) {
            ast_node_array_append(&block->as.block.statements, stmt);
        }
    }
    consume_token(parser, TOKEN_RBRACE, "Expected '}' after block.");
    return block;
}

ASTNode* parse_statement(Parser* parser) {
    if (check_token(parser, TOKEN_LBRACE)) {
        return parse_block(parser);
    }

    if (match_token(parser, TOKEN_KW_IF)) {
        int line = parser->previous.line;
        consume_token(parser, TOKEN_LPAREN, "Expected '(' after 'if'.");
        ASTNode* cond = parse_expression(parser);
        consume_token(parser, TOKEN_RPAREN, "Expected ')' after 'if' condition.");
        ASTNode* then_b = parse_statement(parser);
        ASTNode* else_b = NULL;
        if (match_token(parser, TOKEN_KW_ELSE)) {
            else_b = parse_statement(parser);
        }
        ASTNode* if_node = ast_new_node(NODE_IF, line);
        if_node->as.if_stmt.cond = cond;
        if_node->as.if_stmt.then_b = then_b;
        if_node->as.if_stmt.else_b = else_b;
        return if_node;
    }

    if (match_token(parser, TOKEN_KW_WHILE)) {
        int line = parser->previous.line;
        consume_token(parser, TOKEN_LPAREN, "Expected '(' after 'while'.");
        ASTNode* cond = parse_expression(parser);
        consume_token(parser, TOKEN_RPAREN, "Expected ')' after 'while' condition.");
        ASTNode* body = parse_statement(parser);
        ASTNode* while_node = ast_new_node(NODE_WHILE, line);
        while_node->as.while_stmt.cond = cond;
        while_node->as.while_stmt.body = body;
        return while_node;
    }

    if (match_token(parser, TOKEN_KW_DO)) {
        int line = parser->previous.line;
        ASTNode* body = parse_statement(parser);
        consume_token(parser, TOKEN_KW_WHILE, "Expected 'while' after 'do' body.");
        consume_token(parser, TOKEN_LPAREN, "Expected '(' after 'while'.");
        ASTNode* cond = parse_expression(parser);
        consume_token(parser, TOKEN_RPAREN, "Expected ')' after 'while' condition.");
        match_token(parser, TOKEN_SEMICOLON);
        ASTNode* do_node = ast_new_node(NODE_DO_WHILE, line);
        do_node->as.do_while.cond = cond;
        do_node->as.do_while.body = body;
        return do_node;
    }

    if (match_token(parser, TOKEN_KW_SWITCH)) {
        int line = parser->previous.line;
        if (is_identifier_token(parser->current.type) && slice_equal_case(parser->current.start, parser->current.length, "on", 2)) {
            advance_token(parser);
        }
        ASTNode* target = parse_expression(parser);
        consume_token(parser, TOKEN_LBRACE, "Expected '{' after switch expression.");
        SwitchCase cases[32];
        int case_count = 0;
        while (!check_token(parser, TOKEN_RBRACE) && !check_token(parser, TOKEN_EOF)) {
            consume_token(parser, TOKEN_KW_WHEN, "Expected 'when' in switch statement.");
            ASTNode* match_val = NULL;
            if (match_token(parser, TOKEN_KW_ELSE)) {
                match_val = NULL;
            } else {
                match_val = parse_expression(parser);
            }
            ASTNode* body = parse_statement(parser);
            cases[case_count].match_val = match_val;
            cases[case_count].body = body;
            case_count++;
        }
        consume_token(parser, TOKEN_RBRACE, "Expected '}' after switch block.");
        ASTNode* sw = ast_new_node(NODE_SWITCH, line);
        sw->as.switch_stmt.target = target;
        sw->as.switch_stmt.cases = (SwitchCase*)malloc(sizeof(SwitchCase) * (case_count > 0 ? case_count : 1));
        memcpy(sw->as.switch_stmt.cases, cases, sizeof(SwitchCase) * case_count);
        sw->as.switch_stmt.case_count = case_count;
        return sw;
    }

    if (match_token(parser, TOKEN_KW_TRY)) {
        int line = parser->previous.line;
        ASTNode* try_block = parse_block(parser);
        CatchClause catches[16];
        int catch_count = 0;
        while (match_token(parser, TOKEN_KW_CATCH)) {
            consume_token(parser, TOKEN_LPAREN, "Expected '(' after 'catch'.");
            char* exc_type = parse_type_name(parser);
            char* exc_var = consume_identifier_name(parser, "Expected exception variable name.");
            consume_token(parser, TOKEN_RPAREN, "Expected ')' after catch parameter.");
            ASTNode* c_body = parse_block(parser);
            catches[catch_count].exception_type = exc_type;
            catches[catch_count].var_name = exc_var;
            catches[catch_count].body = c_body;
            catch_count++;
        }
        ASTNode* fin_block = NULL;
        if (match_token(parser, TOKEN_KW_FINALLY)) {
            fin_block = parse_block(parser);
        }
        ASTNode* tc = ast_new_node(NODE_TRY_CATCH, line);
        tc->as.try_catch.try_block = try_block;
        tc->as.try_catch.catches = (CatchClause*)malloc(sizeof(CatchClause) * (catch_count > 0 ? catch_count : 1));
        memcpy(tc->as.try_catch.catches, catches, sizeof(CatchClause) * catch_count);
        tc->as.try_catch.catch_count = catch_count;
        tc->as.try_catch.finally_block = fin_block;
        return tc;
    }

    if (match_token(parser, TOKEN_KW_BREAK) || match_token(parser, TOKEN_KW_CONTINUE)) {
        match_token(parser, TOKEN_SEMICOLON);
        return NULL;
    }

    if (match_token(parser, TOKEN_KW_THROW)) {
        parse_expression(parser);
        match_token(parser, TOKEN_SEMICOLON);
        return NULL;
    }

    if (match_token(parser, TOKEN_KW_FOR)) {
        int line = parser->previous.line;
        consume_token(parser, TOKEN_LPAREN, "Expected '(' after 'for'.");

        if (is_type_token(parser->current.type) || is_identifier_token(parser->current.type)) {
            const char* saved_start = parser->lexer->start;
            const char* saved_cur = parser->lexer->current;
            int saved_line = parser->lexer->line;
            int saved_col = parser->lexer->column;
            Token saved_current = parser->current;
            Token saved_prev = parser->previous;

            char* item_type = parse_type_name(parser);
            if (is_identifier_token(parser->current.type)) {
                char* item_name = duplicate_slice(parser->current.start, parser->current.length);
                advance_token(parser);
                if (match_token(parser, TOKEN_COLON)) {
                    ASTNode* col = parse_expression(parser);
                    consume_token(parser, TOKEN_RPAREN, "Expected ')' after for-each header.");
                    ASTNode* body = parse_statement(parser);
                    ASTNode* for_each = ast_new_node(NODE_FOR_EACH, line);
                    for_each->as.for_each.item_type = item_type;
                    for_each->as.for_each.item_name = item_name;
                    for_each->as.for_each.collection = col;
                    for_each->as.for_each.body = body;
                    return for_each;
                } else {
                    free(item_name);
                }
            }
            free(item_type);
            parser->lexer->start = saved_start;
            parser->lexer->current = saved_cur;
            parser->lexer->line = saved_line;
            parser->lexer->column = saved_col;
            parser->current = saved_current;
            parser->previous = saved_prev;
        }

        ASTNode* init = NULL;
        if (!check_token(parser, TOKEN_SEMICOLON)) {
            if (is_type_token(parser->current.type) || check_token(parser, TOKEN_IDENTIFIER)) {
                char* t_name = parse_type_name(parser);
                consume_token(parser, TOKEN_IDENTIFIER, "Expected loop variable name.");
                char* v_name = duplicate_slice(parser->previous.start, parser->previous.length);
                ASTNode* v_init = NULL;
                if (match_token(parser, TOKEN_ASSIGN)) {
                    v_init = parse_expression(parser);
                }
                init = ast_new_node(NODE_VAR_DECL, line);
                init->as.var_decl.type_name = t_name;
                init->as.var_decl.var_name = v_name;
                init->as.var_decl.init = v_init;
            } else {
                init = parse_expression(parser);
            }
        }
        consume_token(parser, TOKEN_SEMICOLON, "Expected ';' after for-init.");

        ASTNode* cond = NULL;
        if (!check_token(parser, TOKEN_SEMICOLON)) {
            cond = parse_expression(parser);
        }
        consume_token(parser, TOKEN_SEMICOLON, "Expected ';' after for-condition.");

        ASTNode* update = NULL;
        if (!check_token(parser, TOKEN_RPAREN)) {
            update = parse_expression(parser);
        }
        consume_token(parser, TOKEN_RPAREN, "Expected ')' after for-clauses.");

        ASTNode* body = parse_statement(parser);
        ASTNode* for_node = ast_new_node(NODE_FOR, line);
        for_node->as.for_stmt.init = init;
        for_node->as.for_stmt.cond = cond;
        for_node->as.for_stmt.update = update;
        for_node->as.for_stmt.body = body;
        return for_node;
    }

    if (match_token(parser, TOKEN_KW_RETURN)) {
        int line = parser->previous.line;
        ASTNode* val = NULL;
        if (!check_token(parser, TOKEN_SEMICOLON)) {
            val = parse_expression(parser);
        }
        match_token(parser, TOKEN_SEMICOLON);
        ASTNode* ret = ast_new_node(NODE_RETURN, line);
        ret->as.return_stmt.value = val;
        return ret;
    }

    ASTNode* expr = parse_expression(parser);
    match_token(parser, TOKEN_SEMICOLON);
    return expr;
}

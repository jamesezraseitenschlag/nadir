// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#include "parser_internal.h"

ASTNode* parse_class_declaration(Parser* parser) {
    int line = parser->previous.line;
    consume_token(parser, TOKEN_IDENTIFIER, "Expected class name.");
    char* class_name = duplicate_slice(parser->previous.start, parser->previous.length);

    char* parent_class = NULL;
    if (match_token(parser, TOKEN_KW_EXTENDS)) {
        parent_class = parse_type_name(parser);
    }

    if (match_token(parser, TOKEN_KW_IMPLEMENTS)) {
        char* iface = parse_type_name(parser);
        free(iface);
        while (match_token(parser, TOKEN_COMMA)) {
            char* next_iface = parse_type_name(parser);
            free(next_iface);
        }
    }

    consume_token(parser, TOKEN_LBRACE, "Expected '{' before class body.");

    ASTNode* class_node = ast_new_node(NODE_CLASS_DECL, line);
    class_node->as.class_decl.name = class_name;
    class_node->as.class_decl.parent_class = parent_class;

    while (!check_token(parser, TOKEN_RBRACE) && !check_token(parser, TOKEN_EOF)) {
        skip_annotations(parser);

        bool is_static = false;
        while (match_token(parser, TOKEN_KW_PUBLIC) || match_token(parser, TOKEN_KW_PRIVATE) ||
               match_token(parser, TOKEN_KW_GLOBAL) || match_token(parser, TOKEN_KW_PROTECTED) ||
               match_token(parser, TOKEN_KW_STATIC) || match_token(parser, TOKEN_KW_OVERRIDE) ||
               match_token(parser, TOKEN_KW_VIRTUAL) || match_token(parser, TOKEN_KW_ABSTRACT) ||
               match_token(parser, TOKEN_KW_FINAL) || match_token(parser, TOKEN_KW_TRANSIENT) ||
               match_token(parser, TOKEN_KW_TESTMETHOD) || match_token(parser, TOKEN_KW_WEBSERVICE)) {
            if (parser->previous.type == TOKEN_KW_STATIC) is_static = true;
        }

        if (is_static && check_token(parser, TOKEN_LBRACE)) {
            ASTNode* block = parse_block(parser);
            ast_node_array_append(&class_node->as.class_decl.members, block);
            continue;
        }

        if (match_token(parser, TOKEN_KW_CLASS)) {
            ASTNode* inner_class = parse_class_declaration(parser);
            ast_node_array_append(&class_node->as.class_decl.members, inner_class);
            continue;
        }

        if (match_token(parser, TOKEN_KW_INTERFACE)) {
            consume_identifier_name(parser, "Expected interface name.");
            if (match_token(parser, TOKEN_KW_EXTENDS)) {
                consume_identifier_name(parser, "Expected parent interface name.");
            }
            consume_token(parser, TOKEN_LBRACE, "Expected '{' after interface name.");
            while (!check_token(parser, TOKEN_RBRACE) && !check_token(parser, TOKEN_EOF)) {
                parse_type_name(parser);
                consume_identifier_name(parser, "Expected method name.");
                if (match_token(parser, TOKEN_LPAREN)) {
                    while (!check_token(parser, TOKEN_RPAREN) && !check_token(parser, TOKEN_EOF)) {
                        advance_token(parser);
                    }
                    consume_token(parser, TOKEN_RPAREN, "Expected ')' after params.");
                }
                match_token(parser, TOKEN_SEMICOLON);
            }
            consume_token(parser, TOKEN_RBRACE, "Expected '}' after interface body.");
            continue;
        }

        if (match_token(parser, TOKEN_KW_ENUM)) {
            consume_identifier_name(parser, "Expected enum name.");
            consume_token(parser, TOKEN_LBRACE, "Expected '{' after enum name.");
            while (!check_token(parser, TOKEN_RBRACE) && !check_token(parser, TOKEN_EOF)) {
                consume_identifier_name(parser, "Expected enum value.");
                match_token(parser, TOKEN_COMMA);
            }
            consume_token(parser, TOKEN_RBRACE, "Expected '}' after enum body.");
            continue;
        }

        // ctor check
        if (is_identifier_token(parser->current.type) && slice_equal_case(parser->current.start, parser->current.length, class_name, (int)strlen(class_name))) {
            const char* saved_start = parser->lexer->start;
            const char* saved_cur = parser->lexer->current;
            int saved_line = parser->lexer->line;
            int saved_col = parser->lexer->column;
            Token saved_current = parser->current;
            Token saved_prev = parser->previous;

            advance_token(parser);
            if (check_token(parser, TOKEN_LPAREN)) {
                consume_token(parser, TOKEN_LPAREN, "Expected '(' after constructor name.");
                ParamDecl params[16];
                int p_count = 0;
                if (!check_token(parser, TOKEN_RPAREN)) {
                    do {
                        char* p_type = parse_type_name(parser);
                        char* p_name = consume_identifier_name(parser, "Expected parameter name.");
                        params[p_count].type_name = p_type;
                        params[p_count].param_name = p_name;
                        p_count++;
                    } while (match_token(parser, TOKEN_COMMA));
                }
                consume_token(parser, TOKEN_RPAREN, "Expected ')' after parameters.");
                ASTNode* body = parse_block(parser);

                ASTNode* method = ast_new_node(NODE_METHOD_DECL, line);
                method->as.method_decl.name = duplicate_string(class_name);
                method->as.method_decl.return_type = duplicate_string("void");
                method->as.method_decl.params = (ParamDecl*)malloc(sizeof(ParamDecl) * (p_count > 0 ? p_count : 1));
                memcpy(method->as.method_decl.params, params, sizeof(ParamDecl) * p_count);
                method->as.method_decl.param_count = p_count;
                method->as.method_decl.body = body;
                method->as.method_decl.is_static = false;

                ast_node_array_append(&class_node->as.class_decl.members, method);
                continue;
            } else {
                parser->lexer->start = saved_start;
                parser->lexer->current = saved_cur;
                parser->lexer->line = saved_line;
                parser->lexer->column = saved_col;
                parser->current = saved_current;
                parser->previous = saved_prev;
            }
        }

        char* type_name = parse_type_name(parser);
        char* member_name = consume_identifier_name(parser, "Expected member name.");

        if (match_token(parser, TOKEN_LPAREN)) {
            ParamDecl params[16];
            int p_count = 0;
            if (!check_token(parser, TOKEN_RPAREN)) {
                do {
                    char* p_type = parse_type_name(parser);
                    char* p_name = consume_identifier_name(parser, "Expected parameter name.");
                    params[p_count].type_name = p_type;
                    params[p_count].param_name = p_name;
                    p_count++;
                } while (match_token(parser, TOKEN_COMMA));
            }
            consume_token(parser, TOKEN_RPAREN, "Expected ')' after parameters.");
            ASTNode* body = NULL;
            if (match_token(parser, TOKEN_SEMICOLON)) {
                body = NULL;
            } else {
                body = parse_block(parser);
            }

            ASTNode* method = ast_new_node(NODE_METHOD_DECL, line);
            method->as.method_decl.name = member_name;
            method->as.method_decl.return_type = type_name;
            method->as.method_decl.params = (ParamDecl*)malloc(sizeof(ParamDecl) * (p_count > 0 ? p_count : 1));
            memcpy(method->as.method_decl.params, params, sizeof(ParamDecl) * p_count);
            method->as.method_decl.param_count = p_count;
            method->as.method_decl.body = body;
            method->as.method_decl.is_static = is_static;

            ast_node_array_append(&class_node->as.class_decl.members, method);
        } else if (match_token(parser, TOKEN_LBRACE)) {
            int brace_depth = 1;
            while (brace_depth > 0 && !check_token(parser, TOKEN_EOF)) {
                if (match_token(parser, TOKEN_LBRACE)) brace_depth++;
                else if (match_token(parser, TOKEN_RBRACE)) brace_depth--;
                else advance_token(parser);
            }
            ASTNode* field_node = ast_new_node(NODE_VAR_DECL, line);
            field_node->as.var_decl.type_name = type_name;
            field_node->as.var_decl.var_name = member_name;
            field_node->as.var_decl.hash = nadr_hash_str(member_name);
            field_node->as.var_decl.init = NULL;
            ast_node_array_append(&class_node->as.class_decl.members, field_node);
        } else {
            ASTNode* init = NULL;
            if (match_token(parser, TOKEN_ASSIGN)) {
                init = parse_expression(parser);
            }
            match_token(parser, TOKEN_SEMICOLON);
            ASTNode* field_node = ast_new_node(NODE_VAR_DECL, line);
            field_node->as.var_decl.type_name = type_name;
            field_node->as.var_decl.var_name = member_name;
            field_node->as.var_decl.hash = nadr_hash_str(member_name);
            field_node->as.var_decl.init = init;
            ast_node_array_append(&class_node->as.class_decl.members, field_node);
        }
    }

    consume_token(parser, TOKEN_RBRACE, "Expected '}' after class body.");
    return class_node;
}

ASTNode* parse_declaration(Parser* parser) {
    skip_annotations(parser);

    while (match_token(parser, TOKEN_KW_PUBLIC) || match_token(parser, TOKEN_KW_PRIVATE) ||
           match_token(parser, TOKEN_KW_GLOBAL) || match_token(parser, TOKEN_KW_PROTECTED) ||
           match_token(parser, TOKEN_KW_STATIC) || match_token(parser, TOKEN_KW_FINAL) ||
           match_token(parser, TOKEN_KW_TRANSIENT) || match_token(parser, TOKEN_KW_VIRTUAL) ||
           match_token(parser, TOKEN_KW_ABSTRACT) || match_token(parser, TOKEN_KW_TESTMETHOD) ||
           match_token(parser, TOKEN_KW_WEBSERVICE) || match_token(parser, TOKEN_KW_OVERRIDE)) {
    }

    if (match_token(parser, TOKEN_KW_WITH) || match_token(parser, TOKEN_KW_WITHOUT) || match_token(parser, TOKEN_KW_INHERITED)) {
        match_token(parser, TOKEN_KW_SHARING);
    }

    while (match_token(parser, TOKEN_KW_ABSTRACT) || match_token(parser, TOKEN_KW_VIRTUAL)) {
    }

    if (match_token(parser, TOKEN_KW_CLASS)) {
        return parse_class_declaration(parser);
    }

    if (match_token(parser, TOKEN_KW_TRIGGER)) {
        int line = parser->previous.line;
        char* trig_name = consume_identifier_name(parser, "Expected trigger name.");
        consume_identifier_name(parser, "Expected 'on' after trigger name.");
        char* sobj_name = consume_identifier_name(parser, "Expected SObject name in trigger declaration.");
        consume_token(parser, TOKEN_LPAREN, "Expected '(' before trigger events.");
        char events_buf[256] = {0};
        while (!check_token(parser, TOKEN_RPAREN) && !check_token(parser, TOKEN_EOF)) {
            strncat(events_buf, parser->current.start, parser->current.length);
            strcat(events_buf, " ");
            advance_token(parser);
        }
        consume_token(parser, TOKEN_RPAREN, "Expected ')' after trigger events.");
        ASTNode* body = parse_block(parser);
        ASTNode* trig = ast_new_node(NODE_TRIGGER, line);
        trig->as.trigger.name = trig_name;
        trig->as.trigger.sobject_name = sobj_name;
        trig->as.trigger.events = duplicate_string(events_buf);
        trig->as.trigger.body = body;
        return trig;
    }

    if (match_token(parser, TOKEN_KW_ENUM)) {
        consume_identifier_name(parser, "Expected enum name.");
        consume_token(parser, TOKEN_LBRACE, "Expected '{' after enum name.");
        while (!check_token(parser, TOKEN_RBRACE) && !check_token(parser, TOKEN_EOF)) {
            consume_identifier_name(parser, "Expected enum value.");
            match_token(parser, TOKEN_COMMA);
        }
        consume_token(parser, TOKEN_RBRACE, "Expected '}' after enum body.");
        return NULL;
    }

    if (match_token(parser, TOKEN_KW_INTERFACE)) {
        consume_identifier_name(parser, "Expected interface name.");
        if (match_token(parser, TOKEN_KW_EXTENDS)) {
            consume_identifier_name(parser, "Expected parent interface name.");
        }
        consume_token(parser, TOKEN_LBRACE, "Expected '{' after interface name.");
        while (!check_token(parser, TOKEN_RBRACE) && !check_token(parser, TOKEN_EOF)) {
            parse_type_name(parser);
            consume_identifier_name(parser, "Expected method name.");
            if (match_token(parser, TOKEN_LPAREN)) {
                while (!check_token(parser, TOKEN_RPAREN) && !check_token(parser, TOKEN_EOF)) {
                    advance_token(parser);
                }
                consume_token(parser, TOKEN_RPAREN, "Expected ')' after params.");
            }
            match_token(parser, TOKEN_SEMICOLON);
        }
        consume_token(parser, TOKEN_RBRACE, "Expected '}' after interface body.");
        return NULL;
    }

    if (match_token(parser, TOKEN_KW_DML_INSERT) || match_token(parser, TOKEN_KW_DML_UPDATE) ||
        match_token(parser, TOKEN_KW_DML_UPSERT) || match_token(parser, TOKEN_KW_DML_DELETE) ||
        match_token(parser, TOKEN_KW_DML_UNDELETE) || match_token(parser, TOKEN_KW_DML_MERGE)) {
        TokenType op_type = parser->previous.type;
        const char* op_str = (op_type == TOKEN_KW_DML_INSERT) ? "insert" :
                             (op_type == TOKEN_KW_DML_UPDATE) ? "update" :
                             (op_type == TOKEN_KW_DML_UPSERT) ? "upsert" :
                             (op_type == TOKEN_KW_DML_DELETE) ? "delete" :
                             (op_type == TOKEN_KW_DML_UNDELETE) ? "undelete" : "merge";

        if (is_identifier_token(parser->current.type) && slice_equal_case(parser->current.start, parser->current.length, "as", 2)) {
            advance_token(parser);
            if (is_identifier_token(parser->current.type)) {
                advance_token(parser);
            }
        }

        ASTNode* target = parse_expression(parser);
        match_token(parser, TOKEN_SEMICOLON);
        ASTNode* dml = ast_new_node(NODE_DML, parser->previous.line);
        dml->as.dml.operation = duplicate_string(op_str);
        dml->as.dml.target = target;
        return dml;
    }

    if (match_token(parser, TOKEN_KW_BREAK) || match_token(parser, TOKEN_KW_CONTINUE)) {
        match_token(parser, TOKEN_SEMICOLON);
        return NULL;
    }

    if (check_token(parser, TOKEN_KW_RETURN) || check_token(parser, TOKEN_KW_IF) || check_token(parser, TOKEN_KW_WHILE) ||
        check_token(parser, TOKEN_KW_DO) || check_token(parser, TOKEN_KW_FOR) || check_token(parser, TOKEN_KW_SWITCH) ||
        check_token(parser, TOKEN_KW_TRY) || check_token(parser, TOKEN_KW_THROW)) {
        return parse_statement(parser);
    }

    if (is_type_token(parser->current.type) || is_identifier_token(parser->current.type)) {
        const char* saved_start = parser->lexer->start;
        const char* saved_cur = parser->lexer->current;
        int saved_line = parser->lexer->line;
        int saved_col = parser->lexer->column;
        Token saved_current = parser->current;
        Token saved_prev = parser->previous;

        char* type_name = parse_type_name(parser);
        if (is_identifier_token(parser->current.type)) {
            char* var_name = duplicate_slice(parser->current.start, parser->current.length);
            advance_token(parser);

            ASTNode* init = NULL;
            if (match_token(parser, TOKEN_ASSIGN)) {
                init = parse_expression(parser);
            }
            match_token(parser, TOKEN_SEMICOLON);

            ASTNode* var_decl = ast_new_node(NODE_VAR_DECL, parser->previous.line);
            var_decl->as.var_decl.type_name = type_name;
            var_decl->as.var_decl.var_name = var_name;
            var_decl->as.var_decl.hash = nadr_hash_str(var_name);
            var_decl->as.var_decl.init = init;
            return var_decl;
        } else {
            free(type_name);
            parser->lexer->start = saved_start;
            parser->lexer->current = saved_cur;
            parser->lexer->line = saved_line;
            parser->lexer->column = saved_col;
            parser->current = saved_current;
            parser->previous = saved_prev;
        }
    }

    return parse_statement(parser);
}

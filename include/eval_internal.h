// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#ifndef NADIR_EVAL_INTERNAL_H
#define NADIR_EVAL_INTERNAL_H

#include "eval.h"
#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ApexClassDef* find_class(Interpreter* interp, const char* name);
ApexMethod* find_method(Interpreter* interp, ApexClassDef* klass, const char* name);

Value eval_binary_op(Interpreter* interp, ASTNode* node, Environment* env);
Value eval_unary_op(Interpreter* interp, ASTNode* node, Environment* env);
Value eval_assign_op(Interpreter* interp, ASTNode* node, Environment* env);

Value eval_system_builtins(Interpreter* interp, ASTNode* node, Environment* env, bool* handled);
Value eval_method_or_call(Interpreter* interp, ASTNode* node, Environment* env);

#endif

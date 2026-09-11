// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#include "eval_internal.h"

Value eval_assign_op(Interpreter* interp, ASTNode* node, Environment* env) {
    Value val = interpreter_eval(interp, node->as.assign.value, env);
    ASTNode* target = node->as.assign.target;
    const char* op = node->as.assign.op;

    if (target->type == NODE_IDENTIFIER) {
        const char* vname = target->as.identifier.name;
        if (strcmp(op, "=") == 0) {
            env_assign(env, vname, val);
        } else {
            Value cur;
            if (env_get(env, vname, &cur)) {
                if (strcmp(op, "+=") == 0) {
                    if (cur.type == VAL_STRING || val.type == VAL_STRING) {
                        char* s1 = val_to_string(cur);
                        char* s2 = val_to_string(val);
                        char* cat = (char*)malloc(strlen(s1) + strlen(s2) + 1);
                        strcpy(cat, s1);
                        strcat(cat, s2);
                        val = val_string(cat);
                        free(s1); free(s2); free(cat);
                    } else if (cur.type == VAL_INT && val.type == VAL_INT) {
                        val = val_int(cur.as.int_val + val.as.int_val);
                    } else if (cur.type == VAL_DOUBLE || val.type == VAL_DOUBLE) {
                        double d1 = cur.type == VAL_DOUBLE ? cur.as.double_val : cur.as.int_val;
                        double d2 = val.type == VAL_DOUBLE ? val.as.double_val : val.as.int_val;
                        val = val_double(d1 + d2);
                    }
                } else if (strcmp(op, "-=") == 0 && cur.type == VAL_INT && val.type == VAL_INT) {
                    val = val_int(cur.as.int_val - val.as.int_val);
                } else if (strcmp(op, "*=") == 0 && cur.type == VAL_INT && val.type == VAL_INT) {
                    val = val_int(cur.as.int_val * val.as.int_val);
                } else if (strcmp(op, "/=") == 0 && cur.type == VAL_INT && val.type == VAL_INT) {
                    val = val_int(val.as.int_val != 0 ? cur.as.int_val / val.as.int_val : 0);
                }
                env_assign(env, vname, val);
            }
        }
        return val;
    } else if (target->type == NODE_MEMBER_ACCESS) {
        Value target_obj = interpreter_eval(interp, target->as.member_access.target, env);
        const char* mem_name = target->as.member_access.member_name;
        if (target_obj.type == VAL_SOBJECT && target_obj.as.sobject_val) {
            sobject_put(target_obj.as.sobject_val, mem_name, val);
        } else if (target_obj.type == VAL_INSTANCE && target_obj.as.instance_val) {
            env_define(target_obj.as.instance_val->fields, mem_name, val);
        }
        return val;
    } else if (target->type == NODE_CALL && string_equal_case(target->as.call.method_name, "get") && target->as.call.args.count == 1) {
        Value target_col = interpreter_eval(interp, target->as.call.callee, env);
        Value index_val = interpreter_eval(interp, target->as.call.args.nodes[0], env);
        if (target_col.type == VAL_LIST && target_col.as.list_val) {
            int idx = (int)index_val.as.int_val;
            if (idx >= 0 && idx < target_col.as.list_val->count) {
                target_col.as.list_val->items[idx] = val;
            }
        } else if (target_col.type == VAL_MAP && target_col.as.map_val) {
            val_map_put(&target_col, index_val, val);
        }
        return val;
    }
    return val_null();
}

Value eval_binary_op(Interpreter* interp, ASTNode* node, Environment* env) {
    Value left = interpreter_eval(interp, node->as.binary.left, env);
    Value right = interpreter_eval(interp, node->as.binary.right, env);
    const char* op = node->as.binary.op;

    if (strcmp(op, "+") == 0) {
        if (left.type == VAL_STRING || right.type == VAL_STRING) {
            char* s1 = val_to_string(left);
            char* s2 = val_to_string(right);
            char* cat = (char*)malloc(strlen(s1) + strlen(s2) + 1);
            strcpy(cat, s1);
            strcat(cat, s2);
            Value res = val_string(cat);
            free(s1); free(s2); free(cat);
            return res;
        }
        if (left.type == VAL_INT && right.type == VAL_INT) {
            return val_int(left.as.int_val + right.as.int_val);
        }
        if (left.type == VAL_DOUBLE || right.type == VAL_DOUBLE) {
            double d1 = (left.type == VAL_DOUBLE) ? left.as.double_val : (double)left.as.int_val;
            double d2 = (right.type == VAL_DOUBLE) ? right.as.double_val : (double)right.as.int_val;
            return val_double(d1 + d2);
        }
    }

    if (strcmp(op, "-") == 0) {
        if (left.type == VAL_INT && right.type == VAL_INT) return val_int(left.as.int_val - right.as.int_val);
        if (left.type == VAL_DOUBLE || right.type == VAL_DOUBLE) {
            double d1 = (left.type == VAL_DOUBLE) ? left.as.double_val : (double)left.as.int_val;
            double d2 = (right.type == VAL_DOUBLE) ? right.as.double_val : (double)right.as.int_val;
            return val_double(d1 - d2);
        }
    }

    if (strcmp(op, "*") == 0) {
        if (left.type == VAL_INT && right.type == VAL_INT) return val_int(left.as.int_val * right.as.int_val);
        if (left.type == VAL_DOUBLE || right.type == VAL_DOUBLE) {
            double d1 = (left.type == VAL_DOUBLE) ? left.as.double_val : (double)left.as.int_val;
            double d2 = (right.type == VAL_DOUBLE) ? right.as.double_val : (double)right.as.int_val;
            return val_double(d1 * d2);
        }
    }

    if (strcmp(op, "/") == 0) {
        if (left.type == VAL_INT && right.type == VAL_INT) {
            return val_int(right.as.int_val != 0 ? left.as.int_val / right.as.int_val : 0);
        }
        if (left.type == VAL_DOUBLE || right.type == VAL_DOUBLE) {
            double d1 = (left.type == VAL_DOUBLE) ? left.as.double_val : (double)left.as.int_val;
            double d2 = (right.type == VAL_DOUBLE) ? right.as.double_val : (double)right.as.int_val;
            return val_double(d2 != 0.0 ? d1 / d2 : 0.0);
        }
    }

    if (strcmp(op, "%") == 0 && left.type == VAL_INT && right.type == VAL_INT) {
        return val_int(right.as.int_val != 0 ? left.as.int_val % right.as.int_val : 0);
    }

    if (strcmp(op, "==") == 0 || strcmp(op, "===") == 0) return val_bool(val_equals(left, right));
    if (strcmp(op, "!=") == 0 || strcmp(op, "!==") == 0 || strcmp(op, "<>") == 0) return val_bool(!val_equals(left, right));

    if (strcmp(op, "<") == 0) {
        if (left.type == VAL_INT && right.type == VAL_INT) return val_bool(left.as.int_val < right.as.int_val);
        if (left.type == VAL_DOUBLE || right.type == VAL_DOUBLE) {
            double d1 = (left.type == VAL_DOUBLE) ? left.as.double_val : (double)left.as.int_val;
            double d2 = (right.type == VAL_DOUBLE) ? right.as.double_val : (double)right.as.int_val;
            return val_bool(d1 < d2);
        }
    }
    if (strcmp(op, ">") == 0) {
        if (left.type == VAL_INT && right.type == VAL_INT) return val_bool(left.as.int_val > right.as.int_val);
        if (left.type == VAL_DOUBLE || right.type == VAL_DOUBLE) {
            double d1 = (left.type == VAL_DOUBLE) ? left.as.double_val : (double)left.as.int_val;
            double d2 = (right.type == VAL_DOUBLE) ? right.as.double_val : (double)right.as.int_val;
            return val_bool(d1 > d2);
        }
    }
    if (strcmp(op, "<=") == 0) {
        if (left.type == VAL_INT && right.type == VAL_INT) return val_bool(left.as.int_val <= right.as.int_val);
        if (left.type == VAL_DOUBLE || right.type == VAL_DOUBLE) {
            double d1 = (left.type == VAL_DOUBLE) ? left.as.double_val : (double)left.as.int_val;
            double d2 = (right.type == VAL_DOUBLE) ? right.as.double_val : (double)right.as.int_val;
            return val_bool(d1 <= d2);
        }
    }
    if (strcmp(op, ">=") == 0) {
        if (left.type == VAL_INT && right.type == VAL_INT) return val_bool(left.as.int_val >= right.as.int_val);
        if (left.type == VAL_DOUBLE || right.type == VAL_DOUBLE) {
            double d1 = (left.type == VAL_DOUBLE) ? left.as.double_val : (double)left.as.int_val;
            double d2 = (right.type == VAL_DOUBLE) ? right.as.double_val : (double)right.as.int_val;
            return val_bool(d1 >= d2);
        }
    }

    if (strcmp(op, "&&") == 0) return val_bool(val_is_truthy(left) && val_is_truthy(right));
    if (strcmp(op, "||") == 0) return val_bool(val_is_truthy(left) || val_is_truthy(right));

    if (strcmp(op, "??") == 0) {
        return (left.type != VAL_NULL) ? left : right;
    }

    return val_null();
}

Value eval_unary_op(Interpreter* interp, ASTNode* node, Environment* env) {
    const char* op = node->as.unary.op;
    if (strcmp(op, "!") == 0) {
        Value operand = interpreter_eval(interp, node->as.unary.operand, env);
        return val_bool(!val_is_truthy(operand));
    }
    if (strcmp(op, "-") == 0) {
        Value operand = interpreter_eval(interp, node->as.unary.operand, env);
        if (operand.type == VAL_INT) return val_int(-operand.as.int_val);
        if (operand.type == VAL_DOUBLE) return val_double(-operand.as.double_val);
        return val_null();
    }
    if (strcmp(op, "++") == 0) {
        if (node->as.unary.operand->type == NODE_IDENTIFIER) {
            const char* var_name = node->as.unary.operand->as.identifier.name;
            Value cur;
            if (env_get(env, var_name, &cur) && cur.type == VAL_INT) {
                Value n = val_int(cur.as.int_val + 1);
                env_assign(env, var_name, n);
                return node->as.unary.prefix ? n : cur;
            }
        }
    }
    if (strcmp(op, "--") == 0) {
        if (node->as.unary.operand->type == NODE_IDENTIFIER) {
            const char* var_name = node->as.unary.operand->as.identifier.name;
            Value cur;
            if (env_get(env, var_name, &cur) && cur.type == VAL_INT) {
                Value n = val_int(cur.as.int_val - 1);
                env_assign(env, var_name, n);
                return node->as.unary.prefix ? n : cur;
            }
        }
    }
    return val_null();
}

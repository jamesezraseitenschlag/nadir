// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#include "eval_internal.h"

static Value concat_two_values(Value left, Value right) {
    char stack_buf[512];
    char* s1_alloc = NULL;
    char* s2_alloc = NULL;
    const char* s1 = NULL;
    const char* s2 = NULL;
    char num1[32], num2[32];

    if (left.type == VAL_STRING) {
        s1 = left.as.string_val ? left.as.string_val : "";
    } else if (left.type == VAL_INT) {
        snprintf(num1, sizeof(num1), "%lld", (long long)left.as.int_val);
        s1 = num1;
    } else if (left.type == VAL_DOUBLE) {
        snprintf(num1, sizeof(num1), "%.6g", left.as.double_val);
        s1 = num1;
    } else if (left.type == VAL_BOOL) {
        s1 = left.as.bool_val ? "true" : "false";
    } else if (left.type == VAL_NULL) {
        s1 = "null";
    } else {
        s1_alloc = val_to_string(left);
        s1 = s1_alloc;
    }

    if (right.type == VAL_STRING) {
        s2 = right.as.string_val ? right.as.string_val : "";
    } else if (right.type == VAL_INT) {
        snprintf(num2, sizeof(num2), "%lld", (long long)right.as.int_val);
        s2 = num2;
    } else if (right.type == VAL_DOUBLE) {
        snprintf(num2, sizeof(num2), "%.6g", right.as.double_val);
        s2 = num2;
    } else if (right.type == VAL_BOOL) {
        s2 = right.as.bool_val ? "true" : "false";
    } else if (right.type == VAL_NULL) {
        s2 = "null";
    } else {
        s2_alloc = val_to_string(right);
        s2 = s2_alloc;
    }

    size_t l1 = strlen(s1);
    size_t l2 = strlen(s2);
    size_t total = l1 + l2;

    Value res;
    if (total < sizeof(stack_buf)) {
        memcpy(stack_buf, s1, l1);
        memcpy(stack_buf + l1, s2, l2);
        stack_buf[total] = '\0';
        res = val_string_slice(stack_buf, (int)total);
    } else {
        char* heap = (char*)malloc(total + 1);
        if (!heap) abort();
        memcpy(heap, s1, l1);
        memcpy(heap + l1, s2, l2);
        heap[total] = '\0';
        res.type = VAL_STRING;
        res.as.string_val = heap;
    }

    if (s1_alloc) free(s1_alloc);
    if (s2_alloc) free(s2_alloc);
    return res;
}

Value eval_assign_op(Interpreter* interp, ASTNode* node, Environment* env) {
    Value val = interpreter_eval(interp, node->as.assign.value, env);
    ASTNode* target = node->as.assign.target;
    AssignOpKind op_kind = node->as.assign.op_kind;

    if (target->type == NODE_IDENTIFIER) {
        const char* vname = target->as.identifier.name;
        uint32_t vhash = target->as.identifier.hash;
        if (vhash == 0 && vname) vhash = nadr_hash_str(vname);

        if (op_kind == OP_ASSIGN) {
            env_assign_prehashed(env, vname, vhash, val);
        } else {
            Value cur;
            if (env_get_prehashed(env, vname, vhash, &cur)) {
                switch (op_kind) {
                    case OP_PLUS_ASSIGN:
                        if (cur.type == VAL_INT && val.type == VAL_INT) {
                            val = val_int(cur.as.int_val + val.as.int_val);
                        } else if (cur.type == VAL_STRING || val.type == VAL_STRING) {
                            val = concat_two_values(cur, val);
                        } else if (cur.type == VAL_DOUBLE || val.type == VAL_DOUBLE) {
                            double d1 = cur.type == VAL_DOUBLE ? cur.as.double_val : (double)cur.as.int_val;
                            double d2 = val.type == VAL_DOUBLE ? val.as.double_val : (double)val.as.int_val;
                            val = val_double(d1 + d2);
                        }
                        break;
                    case OP_MINUS_ASSIGN:
                        if (cur.type == VAL_INT && val.type == VAL_INT) val = val_int(cur.as.int_val - val.as.int_val);
                        break;
                    case OP_STAR_ASSIGN:
                        if (cur.type == VAL_INT && val.type == VAL_INT) val = val_int(cur.as.int_val * val.as.int_val);
                        break;
                    case OP_SLASH_ASSIGN:
                        if (cur.type == VAL_INT && val.type == VAL_INT) val = val_int(val.as.int_val != 0 ? cur.as.int_val / val.as.int_val : 0);
                        break;
                    default:
                        break;
                }
                env_assign_prehashed(env, vname, vhash, val);
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
    BinaryOpKind op_kind = node->as.binary.op_kind;

    switch (op_kind) {
        case BINOP_ADD:
            if (left.type == VAL_INT && right.type == VAL_INT) {
                return val_int(left.as.int_val + right.as.int_val);
            }
            if (left.type == VAL_STRING || right.type == VAL_STRING) {
                return concat_two_values(left, right);
            }
            if (left.type == VAL_DOUBLE || right.type == VAL_DOUBLE) {
                double d1 = (left.type == VAL_DOUBLE) ? left.as.double_val : (double)left.as.int_val;
                double d2 = (right.type == VAL_DOUBLE) ? right.as.double_val : (double)right.as.int_val;
                return val_double(d1 + d2);
            }
            return val_null();

        case BINOP_SUB:
            if (left.type == VAL_INT && right.type == VAL_INT) return val_int(left.as.int_val - right.as.int_val);
            if (left.type == VAL_DOUBLE || right.type == VAL_DOUBLE) {
                double d1 = (left.type == VAL_DOUBLE) ? left.as.double_val : (double)left.as.int_val;
                double d2 = (right.type == VAL_DOUBLE) ? right.as.double_val : (double)right.as.int_val;
                return val_double(d1 - d2);
            }
            return val_null();

        case BINOP_MUL:
            if (left.type == VAL_INT && right.type == VAL_INT) return val_int(left.as.int_val * right.as.int_val);
            if (left.type == VAL_DOUBLE || right.type == VAL_DOUBLE) {
                double d1 = (left.type == VAL_DOUBLE) ? left.as.double_val : (double)left.as.int_val;
                double d2 = (right.type == VAL_DOUBLE) ? right.as.double_val : (double)right.as.int_val;
                return val_double(d1 * d2);
            }
            return val_null();

        case BINOP_DIV:
            if (left.type == VAL_INT && right.type == VAL_INT) {
                return val_int(right.as.int_val != 0 ? left.as.int_val / right.as.int_val : 0);
            }
            if (left.type == VAL_DOUBLE || right.type == VAL_DOUBLE) {
                double d1 = (left.type == VAL_DOUBLE) ? left.as.double_val : (double)left.as.int_val;
                double d2 = (right.type == VAL_DOUBLE) ? right.as.double_val : (double)right.as.int_val;
                return val_double(d2 != 0.0 ? d1 / d2 : 0.0);
            }
            return val_null();

        case BINOP_MOD:
            if (left.type == VAL_INT && right.type == VAL_INT) {
                return val_int(right.as.int_val != 0 ? left.as.int_val % right.as.int_val : 0);
            }
            return val_null();

        case BINOP_EQ:
            return val_bool(val_equals(left, right));

        case BINOP_NE:
            return val_bool(!val_equals(left, right));

        case BINOP_LT:
            if (left.type == VAL_INT && right.type == VAL_INT) return val_bool(left.as.int_val < right.as.int_val);
            if (left.type == VAL_DOUBLE || right.type == VAL_DOUBLE) {
                double d1 = (left.type == VAL_DOUBLE) ? left.as.double_val : (double)left.as.int_val;
                double d2 = (right.type == VAL_DOUBLE) ? right.as.double_val : (double)right.as.int_val;
                return val_bool(d1 < d2);
            }
            return val_bool(false);

        case BINOP_LE:
            if (left.type == VAL_INT && right.type == VAL_INT) return val_bool(left.as.int_val <= right.as.int_val);
            if (left.type == VAL_DOUBLE || right.type == VAL_DOUBLE) {
                double d1 = (left.type == VAL_DOUBLE) ? left.as.double_val : (double)left.as.int_val;
                double d2 = (right.type == VAL_DOUBLE) ? right.as.double_val : (double)right.as.int_val;
                return val_bool(d1 <= d2);
            }
            return val_bool(false);

        case BINOP_GT:
            if (left.type == VAL_INT && right.type == VAL_INT) return val_bool(left.as.int_val > right.as.int_val);
            if (left.type == VAL_DOUBLE || right.type == VAL_DOUBLE) {
                double d1 = (left.type == VAL_DOUBLE) ? left.as.double_val : (double)left.as.int_val;
                double d2 = (right.type == VAL_DOUBLE) ? right.as.double_val : (double)right.as.int_val;
                return val_bool(d1 > d2);
            }
            return val_bool(false);

        case BINOP_GE:
            if (left.type == VAL_INT && right.type == VAL_INT) return val_bool(left.as.int_val >= right.as.int_val);
            if (left.type == VAL_DOUBLE || right.type == VAL_DOUBLE) {
                double d1 = (left.type == VAL_DOUBLE) ? left.as.double_val : (double)left.as.int_val;
                double d2 = (right.type == VAL_DOUBLE) ? right.as.double_val : (double)right.as.int_val;
                return val_bool(d1 >= d2);
            }
            return val_bool(false);

        case BINOP_AND:
            return val_bool(val_is_truthy(left) && val_is_truthy(right));

        case BINOP_OR:
            return val_bool(val_is_truthy(left) || val_is_truthy(right));

        case BINOP_NULL_COALESCE:
            return (left.type != VAL_NULL) ? left : right;

        default:
            break;
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
            uint32_t var_hash = node->as.unary.operand->as.identifier.hash;
            if (var_hash == 0 && var_name) var_hash = nadr_hash_str(var_name);
            Value cur;
            if (env_get_prehashed(env, var_name, var_hash, &cur) && cur.type == VAL_INT) {
                Value n = val_int(cur.as.int_val + 1);
                env_assign_prehashed(env, var_name, var_hash, n);
                return node->as.unary.prefix ? n : cur;
            }
        }
    }
    if (strcmp(op, "--") == 0) {
        if (node->as.unary.operand->type == NODE_IDENTIFIER) {
            const char* var_name = node->as.unary.operand->as.identifier.name;
            uint32_t var_hash = node->as.unary.operand->as.identifier.hash;
            if (var_hash == 0 && var_name) var_hash = nadr_hash_str(var_name);
            Value cur;
            if (env_get_prehashed(env, var_name, var_hash, &cur) && cur.type == VAL_INT) {
                Value n = val_int(cur.as.int_val - 1);
                env_assign_prehashed(env, var_name, var_hash, n);
                return node->as.unary.prefix ? n : cur;
            }
        }
    }
    return val_null();
}

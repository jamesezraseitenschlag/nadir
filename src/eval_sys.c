// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#include "eval_internal.h"
#include "metadata.h"
#include "nadir_hash.h"
#include <time.h>
#include <math.h>

Value eval_system_builtins(Interpreter* interp, ASTNode* node, Environment* env, bool* handled) {
    const char* method_name = node->as.call.method_name;
    ASTNode* callee_node = node->as.call.callee;

    if (!callee_node || callee_node->type != NODE_IDENTIFIER) {
        *handled = false;
        return val_null();
    }

    const char* receiver = callee_node->as.identifier.name;

    if (string_equal_case(receiver, "system")) {
        *handled = true;
        if (string_equal_case(method_name, "debug")) {
            if (node->as.call.args.count > 0) {
                Value msg_val = interpreter_eval(interp, node->as.call.args.nodes[0], env);
                printf("[debug] ");
                val_print(msg_val);
                printf("\n");
            }
            return val_null();
        }
        if (string_equal_case(method_name, "assertequals") && node->as.call.args.count >= 2) {
            Value expected = interpreter_eval(interp, node->as.call.args.nodes[0], env);
            Value actual = interpreter_eval(interp, node->as.call.args.nodes[1], env);
            if (!val_equals(expected, actual)) {
                fprintf(stderr, "AssertionException: Expected %s, but was %s\n", val_to_string(expected), val_to_string(actual));
            }
            return val_null();
        }
        if (string_equal_case(method_name, "assertnotequals") && node->as.call.args.count >= 2) {
            Value expected = interpreter_eval(interp, node->as.call.args.nodes[0], env);
            Value actual = interpreter_eval(interp, node->as.call.args.nodes[1], env);
            if (val_equals(expected, actual)) {
                fprintf(stderr, "AssertionException: Values are unexpectedly equal: %s\n", val_to_string(actual));
            }
            return val_null();
        }
        if (string_equal_case(method_name, "assert") && node->as.call.args.count >= 1) {
            Value cond = interpreter_eval(interp, node->as.call.args.nodes[0], env);
            if (!val_is_truthy(cond)) {
                fprintf(stderr, "AssertionException: Condition failed\n");
            }
            return val_null();
        }
        if (string_equal_case(method_name, "now")) {
            time_t now = time(NULL);
            char buf[64];
            strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
            return val_string(buf);
        }
        if (string_equal_case(method_name, "today")) {
            time_t now = time(NULL);
            char buf[64];
            strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&now));
            return val_string(buf);
        }
        if (string_equal_case(method_name, "currenttimemillis")) {
            return val_int(nadir_epoch_ms());
        }
        // monotonic high resolution clock for profiling / benchmarks.
        // currentTimeMillis is a wall clock and can jump; this cannot.
        if (string_equal_case(method_name, "nanotime") || string_equal_case(method_name, "monotonicmillis")) {
            return val_int(nadr_monotonic_ms());
        }
        return val_null();
    }

    if (string_equal_case(receiver, "math")) {
        *handled = true;
        if (string_equal_case(method_name, "max") && node->as.call.args.count >= 2) {
            Value a = interpreter_eval(interp, node->as.call.args.nodes[0], env);
            Value b = interpreter_eval(interp, node->as.call.args.nodes[1], env);
            if (a.type == VAL_DOUBLE || b.type == VAL_DOUBLE) {
                double da = a.type == VAL_DOUBLE ? a.as.double_val : (double)a.as.int_val;
                double db = b.type == VAL_DOUBLE ? b.as.double_val : (double)b.as.int_val;
                return val_double(da > db ? da : db);
            }
            return val_int(a.as.int_val > b.as.int_val ? a.as.int_val : b.as.int_val);
        }
        if (string_equal_case(method_name, "min") && node->as.call.args.count >= 2) {
            Value a = interpreter_eval(interp, node->as.call.args.nodes[0], env);
            Value b = interpreter_eval(interp, node->as.call.args.nodes[1], env);
            if (a.type == VAL_DOUBLE || b.type == VAL_DOUBLE) {
                double da = a.type == VAL_DOUBLE ? a.as.double_val : (double)a.as.int_val;
                double db = b.type == VAL_DOUBLE ? b.as.double_val : (double)b.as.int_val;
                return val_double(da < db ? da : db);
            }
            return val_int(a.as.int_val < b.as.int_val ? a.as.int_val : b.as.int_val);
        }
        if (string_equal_case(method_name, "abs") && node->as.call.args.count >= 1) {
            Value a = interpreter_eval(interp, node->as.call.args.nodes[0], env);
            if (a.type == VAL_DOUBLE) return val_double(fabs(a.as.double_val));
            return val_int(a.as.int_val < 0 ? -a.as.int_val : a.as.int_val);
        }
        if (string_equal_case(method_name, "round") && node->as.call.args.count >= 1) {
            Value a = interpreter_eval(interp, node->as.call.args.nodes[0], env);
            double d = a.type == VAL_DOUBLE ? a.as.double_val : (double)a.as.int_val;
            return val_int((int64_t)round(d));
        }
        if (string_equal_case(method_name, "floor") && node->as.call.args.count >= 1) {
            Value a = interpreter_eval(interp, node->as.call.args.nodes[0], env);
            double d = a.type == VAL_DOUBLE ? a.as.double_val : (double)a.as.int_val;
            return val_int((int64_t)floor(d));
        }
        if (string_equal_case(method_name, "ceil") && node->as.call.args.count >= 1) {
            Value a = interpreter_eval(interp, node->as.call.args.nodes[0], env);
            double d = a.type == VAL_DOUBLE ? a.as.double_val : (double)a.as.int_val;
            return val_int((int64_t)ceil(d));
        }
        if (string_equal_case(method_name, "sqrt") && node->as.call.args.count >= 1) {
            Value a = interpreter_eval(interp, node->as.call.args.nodes[0], env);
            double d = a.type == VAL_DOUBLE ? a.as.double_val : (double)a.as.int_val;
            return val_double(sqrt(d));
        }
        return val_null();
    }

    if (string_equal_case(receiver, "string")) {
        *handled = true;
        if (string_equal_case(method_name, "isblank") && node->as.call.args.count >= 1) {
            Value a = interpreter_eval(interp, node->as.call.args.nodes[0], env);
            return val_bool(a.type == VAL_NULL || (a.type == VAL_STRING && strlen(a.as.string_val) == 0));
        }
        if (string_equal_case(method_name, "isnotblank") && node->as.call.args.count >= 1) {
            Value a = interpreter_eval(interp, node->as.call.args.nodes[0], env);
            return val_bool(a.type == VAL_STRING && strlen(a.as.string_val) > 0);
        }
        if (string_equal_case(method_name, "valueof") && node->as.call.args.count >= 1) {
            Value a = interpreter_eval(interp, node->as.call.args.nodes[0], env);
            return val_string(val_to_string(a));
        }
        return val_null();
    }

    if (string_equal_case(receiver, "json")) {
        *handled = true;
        if ((string_equal_case(method_name, "serialize") || string_equal_case(method_name, "serializepretty")) && node->as.call.args.count >= 1) {
            Value a = interpreter_eval(interp, node->as.call.args.nodes[0], env);
            return val_string(val_to_string(a));
        }
        return val_null();
    }

    if (string_equal_case(receiver, "database")) {
        *handled = true;
        if ((string_equal_case(method_name, "insert") || string_equal_case(method_name, "upsert")) && node->as.call.args.count >= 1) {
            Value target = interpreter_eval(interp, node->as.call.args.nodes[0], env);
            if (target.type == VAL_SOBJECT && target.as.sobject_val) return mock_db_insert(target.as.sobject_val);
            return target;
        }
        if (string_equal_case(method_name, "update") && node->as.call.args.count >= 1) {
            Value target = interpreter_eval(interp, node->as.call.args.nodes[0], env);
            if (target.type == VAL_SOBJECT && target.as.sobject_val) return mock_db_update(target.as.sobject_val);
            return target;
        }
        if (string_equal_case(method_name, "delete") && node->as.call.args.count >= 1) {
            Value target = interpreter_eval(interp, node->as.call.args.nodes[0], env);
            if (target.type == VAL_SOBJECT && target.as.sobject_val) return mock_db_delete(target.as.sobject_val);
            return target;
        }
        if (string_equal_case(method_name, "setsavepoint")) {
            int sp_id = mock_db_set_savepoint();
            return val_int(sp_id);
        }
        if (string_equal_case(method_name, "rollback")) {
            if (node->as.call.args.count >= 1) {
                Value sp_val = interpreter_eval(interp, node->as.call.args.nodes[0], env);
                if (sp_val.type == VAL_INT) {
                    mock_db_rollback((int)sp_val.as.int_val);
                }
            }
            return val_null();
        }
        if (string_equal_case(method_name, "query") && node->as.call.args.count >= 1) {
            return val_list();
        }
        if (string_equal_case(method_name, "countquery") && node->as.call.args.count >= 1) {
            return val_int(0);
        }
        return val_null();
    }

    if (string_equal_case(receiver, "test")) {
        *handled = true;
        if (string_equal_case(method_name, "starttest") || string_equal_case(method_name, "stoptest")) {
            return val_null();
        }
        if (string_equal_case(method_name, "isrunningtest")) {
            return val_bool(true);
        }
        if (string_equal_case(method_name, "setmock")) {
            return val_null();
        }
        return val_null();
    }

    if (string_equal_case(receiver, "limits")) {
        *handled = true;
        if (string_equal_case(method_name, "getqueries")) return val_int(interp->limits.soql_queries);
        if (string_equal_case(method_name, "getlimitqueries")) return val_int(interp->limits.limit_soql_queries);
        if (string_equal_case(method_name, "getdmlstatements")) return val_int(interp->limits.dml_statements);
        if (string_equal_case(method_name, "getlimitdmlstatements")) return val_int(interp->limits.limit_dml_statements);
        if (string_equal_case(method_name, "getdmlrows")) return val_int(interp->limits.dml_rows);
        if (string_equal_case(method_name, "getlimitdmlrows")) return val_int(interp->limits.limit_dml_rows);
        if (string_equal_case(method_name, "getcputime")) return val_int(interp->limits.cpu_time_ms);
        if (string_equal_case(method_name, "getlimitcputime")) return val_int(interp->limits.limit_cpu_time_ms);
        return val_null();
    }

    if (string_equal_case(receiver, "userinfo")) {
        *handled = true;
        if (string_equal_case(method_name, "getuserid")) return val_string("005000000000001AAA");
        if (string_equal_case(method_name, "getusername")) return val_string("admin@local.test");
        if (string_equal_case(method_name, "getorganizationid")) return val_string("00D000000000001AAA");
        if (string_equal_case(method_name, "getorganizationname")) return val_string("Test Org");
        if (string_equal_case(method_name, "getlocale")) return val_string("en_US");
        if (string_equal_case(method_name, "gettimezone")) return val_string("America/Los_Angeles");
        return val_null();
    }

    if (string_equal_case(receiver, "schema")) {
        *handled = true;
        if (string_equal_case(method_name, "getglobaldescribe")) {
            Value map = val_map();
            ProjectSchema* ps = project_schema_get_instance();
            for (int i = 0; i < ps->object_count; i++) {
                val_map_put(&map, val_string(ps->objects[i].full_name), val_string(ps->objects[i].label));
            }
            return map;
        }
        return val_null();
    }

    if (string_equal_case(receiver, "crypto")) {
        *handled = true;
        if (string_equal_case(method_name, "murmur3") && node->as.call.args.count >= 1) {
            Value input = interpreter_eval(interp, node->as.call.args.nodes[0], env);
            const char* s = input.type == VAL_STRING ? input.as.string_val : val_to_string(input);
            uint32_t h = murmur3_32(s, strlen(s), 0x9747b28c);
            char hex[16];
            snprintf(hex, sizeof(hex), "%08x", h);
            return val_string(hex);
        }
        if (string_equal_case(method_name, "murmur3_128") && node->as.call.args.count >= 1) {
            Value input = interpreter_eval(interp, node->as.call.args.nodes[0], env);
            const char* s = input.type == VAL_STRING ? input.as.string_val : val_to_string(input);
            uint8_t out[16];
            murmur3_128(s, strlen(s), 0x9747b28c, out);
            char hex[36];
            for (int i = 0; i < 16; i++) {
                snprintf(hex + i * 2, 3, "%02x", out[i]);
            }
            return val_string(hex);
        }
        if (string_equal_case(method_name, "xxhash32") && node->as.call.args.count >= 1) {
            Value input = interpreter_eval(interp, node->as.call.args.nodes[0], env);
            const char* s = input.type == VAL_STRING ? input.as.string_val : val_to_string(input);
            uint32_t h = xxhash32(s, strlen(s), 0);
            char hex[16];
            snprintf(hex, sizeof(hex), "%08x", h);
            return val_string(hex);
        }
        if (string_equal_case(method_name, "xxhash64") && node->as.call.args.count >= 1) {
            Value input = interpreter_eval(interp, node->as.call.args.nodes[0], env);
            const char* s = input.type == VAL_STRING ? input.as.string_val : val_to_string(input);
            uint64_t h = xxhash64(s, strlen(s), 0);
            char hex[24];
            snprintf(hex, sizeof(hex), "%016llx", (unsigned long long)h);
            return val_string(hex);
        }
        return val_null();
    }

    *handled = false;
    return val_null();
}

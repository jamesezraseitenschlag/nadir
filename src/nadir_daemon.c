// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime api daemon implementation with mongoose

#include "common.h"
#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "eval.h"
#include "sobject.h"
#include "metadata.h"
#include "nadir_daemon.h"

// Protect TokenType from collision with Windows winnt.h
#define TokenType WindowsTokenType
#include "mongoose.h"
#undef TokenType

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Interpreter* g_daemon_interp = NULL;
static bool g_daemon_running = true;
static int g_daemon_port = 8088;

// -----------------------------------------------------------------------------
// SObject Prefix to Type Resolver
// -----------------------------------------------------------------------------

static const char* sobject_type_from_id(const char* id) {
    if (!id || strlen(id) < 3) return "Account";
    if (strncmp(id, "001", 3) == 0) return "Account";
    if (strncmp(id, "003", 3) == 0) return "Contact";
    if (strncmp(id, "006", 3) == 0) return "Opportunity";
    if (strncmp(id, "00Q", 3) == 0) return "Lead";
    if (strncmp(id, "500", 3) == 0) return "Case";
    if (strncmp(id, "801", 3) == 0) return "Order";
    if (strncmp(id, "802", 3) == 0) return "OrderItem";
    if (strncmp(id, "005", 3) == 0) return "User";
    if (strncmp(id, "00e", 3) == 0) return "Profile";
    if (strncmp(id, "00X", 3) == 0) return "EmailTemplate";
    return "Account";
}

// -----------------------------------------------------------------------------
// Dynamic JSON String Buffer
// -----------------------------------------------------------------------------

typedef struct {
    char* data;
    size_t len;
    size_t cap;
} JsonBuffer;

static void jb_init(JsonBuffer* b, size_t initial_cap) {
    if (initial_cap < 256) initial_cap = 256;
    b->data = (char*)malloc(initial_cap);
    if (!b->data) abort();
    b->data[0] = '\0';
    b->len = 0;
    b->cap = initial_cap;
}

static void jb_ensure(JsonBuffer* b, size_t needed) {
    if (b->len + needed + 1 >= b->cap) {
        size_t new_cap = b->cap * 2;
        while (b->len + needed + 1 >= new_cap) new_cap *= 2;
        b->data = (char*)realloc(b->data, new_cap);
        if (!b->data) abort();
        b->cap = new_cap;
    }
}

static void jb_append_str(JsonBuffer* b, const char* str) {
    if (!str) return;
    size_t slen = strlen(str);
    jb_ensure(b, slen);
    memcpy(b->data + b->len, str, slen);
    b->len += slen;
    b->data[b->len] = '\0';
}

static void jb_append_escaped_str(JsonBuffer* b, const char* str) {
    if (!str) {
        jb_append_str(b, "null");
        return;
    }
    jb_ensure(b, 2);
    b->data[b->len++] = '"';
    for (const char* p = str; *p; p++) {
        unsigned char c = (unsigned char)*p;
        switch (c) {
            case '"':  jb_append_str(b, "\\\""); break;
            case '\\': jb_append_str(b, "\\\\"); break;
            case '\b': jb_append_str(b, "\\b"); break;
            case '\f': jb_append_str(b, "\\f"); break;
            case '\n': jb_append_str(b, "\\n"); break;
            case '\r': jb_append_str(b, "\\r"); break;
            case '\t': jb_append_str(b, "\\t"); break;
            default:
                if (c < 32) {
                    char hex[8];
                    snprintf(hex, sizeof(hex), "\\u%04x", c);
                    jb_append_str(b, hex);
                } else {
                    jb_ensure(b, 1);
                    b->data[b->len++] = (char)c;
                }
                break;
        }
    }
    jb_ensure(b, 1);
    b->data[b->len++] = '"';
    b->data[b->len] = '\0';
}

static void val_to_json_rec(Value v, JsonBuffer* b) {
    char num_buf[64];
    switch (v.type) {
        case VAL_NULL:
            jb_append_str(b, "null");
            break;
        case VAL_BOOL:
            jb_append_str(b, v.as.bool_val ? "true" : "false");
            break;
        case VAL_INT:
            snprintf(num_buf, sizeof(num_buf), "%lld", (long long)v.as.int_val);
            jb_append_str(b, num_buf);
            break;
        case VAL_DOUBLE:
            snprintf(num_buf, sizeof(num_buf), "%.6g", v.as.double_val);
            jb_append_str(b, num_buf);
            break;
        case VAL_STRING:
            jb_append_escaped_str(b, v.as.string_val);
            break;
        case VAL_LIST: {
            jb_append_str(b, "[");
            if (v.as.list_val) {
                ValueArray* arr = v.as.list_val;
                for (int i = 0; i < arr->count; i++) {
                    if (i > 0) jb_append_str(b, ", ");
                    val_to_json_rec(arr->items[i], b);
                }
            }
            jb_append_str(b, "]");
            break;
        }
        case VAL_MAP: {
            jb_append_str(b, "{");
            if (v.as.map_val) {
                ValueMap* vm = v.as.map_val;
                bool first = true;
                for (int i = 0; i < vm->capacity; i++) {
                    if (!vm->entries[i].occupied) continue;
                    if (!first) jb_append_str(b, ", ");
                    first = false;
                    Value k = vm->entries[i].key;
                    if (k.type == VAL_STRING) {
                        jb_append_escaped_str(b, k.as.string_val);
                    } else {
                        char kstr[64];
                        snprintf(kstr, sizeof(kstr), "\"key_%d\"", i);
                        jb_append_str(b, kstr);
                    }
                    jb_append_str(b, ": ");
                    val_to_json_rec(vm->entries[i].value, b);
                }
            }
            jb_append_str(b, "}");
            break;
        }
        case VAL_SOBJECT: {
            jb_append_str(b, "{");
            if (v.as.sobject_val) {
                SObject* obj = v.as.sobject_val;
                jb_append_str(b, "\"attributes\": {\"type\": ");
                jb_append_escaped_str(b, obj->type_name ? obj->type_name : "SObject");
                jb_append_str(b, "}");
                for (int i = 0; i < obj->field_count; i++) {
                    if (!obj->fields[i].name) continue;
                    jb_append_str(b, ", ");
                    jb_append_escaped_str(b, obj->fields[i].name);
                    jb_append_str(b, ": ");
                    val_to_json_rec(obj->fields[i].value, b);
                }
            }
            jb_append_str(b, "}");
            break;
        }
        case VAL_INSTANCE: {
            jb_append_str(b, "{");
            if (v.as.instance_val && v.as.instance_val->fields) {
                Environment* env = v.as.instance_val->fields;
                bool first = true;
                for (int i = 0; i < env->count; i++) {
                    if (!env->entries[i].name) continue;
                    if (!first) jb_append_str(b, ", ");
                    first = false;
                    jb_append_escaped_str(b, env->entries[i].name);
                    jb_append_str(b, ": ");
                    val_to_json_rec(env->entries[i].value, b);
                }
            }
            jb_append_str(b, "}");
            break;
        }
        default:
            jb_append_str(b, "null");
            break;
    }
}

char* val_to_json_string(Value v) {
    JsonBuffer b;
    jb_init(&b, 512);
    val_to_json_rec(v, &b);
    return b.data;
}

// -----------------------------------------------------------------------------
// HTTP Helpers & CORS Response
// -----------------------------------------------------------------------------

static const char* CORS_HEADERS =
    "Content-Type: application/json\r\n"
    "Access-Control-Allow-Origin: *\r\n"
    "Access-Control-Allow-Methods: GET, POST, PUT, PATCH, DELETE, OPTIONS\r\n"
    "Access-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With, X-SFDC-Session-Id\r\n"
    "Access-Control-Max-Age: 86400\r\n";

static void send_cors_options(struct mg_connection* c) {
    mg_http_reply(c, 204, CORS_HEADERS, "");
}

static void send_json_response(struct mg_connection* c, int status_code, const char* status_text, const char* json_body) {
    (void)status_text;
    mg_http_reply(c, status_code, CORS_HEADERS, "%s", json_body ? json_body : "");
}

// -----------------------------------------------------------------------------
// @AuraEnabled Gateway Handler
// -----------------------------------------------------------------------------

static Value execute_apex_source(const char* source) {
    if (!g_daemon_interp || !source) return val_null();
    Lexer lexer;
    lexer_init(&lexer, source);
    Parser parser;
    parser_init(&parser, &lexer);
    ASTNode* prog = parser_parse(&parser);
    Value res = val_null();
    if (!parser.had_error && prog) {
        res = interpreter_run(g_daemon_interp, prog);
    } else if (prog) {
        ast_node_free(prog);
    }
    return res;
}

static void handle_aura_gateway(struct mg_connection* c, struct mg_http_message* hm) {
    char class_name[128] = {0};
    char method_name[128] = {0};

    // Parse class and method from JSON payload
    char* cname = mg_json_get_str(hm->body, "$.className");
    if (!cname) cname = mg_json_get_str(hm->body, "$.classname");
    if (!cname) cname = mg_json_get_str(hm->body, "$.actions[0].params.classname");

    char* mname = mg_json_get_str(hm->body, "$.methodName");
    if (!mname) mname = mg_json_get_str(hm->body, "$.method");
    if (!mname) mname = mg_json_get_str(hm->body, "$.actions[0].params.method");

    if (cname) {
        strncpy(class_name, cname, sizeof(class_name) - 1);
        free(cname);
    }
    if (mname) {
        strncpy(method_name, mname, sizeof(method_name) - 1);
        free(mname);
    }

    if (class_name[0] == '\0' || method_name[0] == '\0') {
        send_json_response(c, 400, "Bad Request",
                           "{\"status\":\"ERROR\",\"error\":{\"message\":\"Missing className or methodName\"}}");
        return;
    }

    // Find class and method to format exact parameter list
    ApexClassDef* klass = find_class(g_daemon_interp, class_name);
    ApexMethod* method = klass ? find_method(g_daemon_interp, klass, method_name) : NULL;

    JsonBuffer inv_jb;
    jb_init(&inv_jb, 512);
    jb_append_str(&inv_jb, class_name);
    jb_append_str(&inv_jb, ".");
    jb_append_str(&inv_jb, method_name);
    jb_append_str(&inv_jb, "(");

    if (method && method->param_count > 0) {
        for (int p = 0; p < method->param_count; p++) {
            if (p > 0) jb_append_str(&inv_jb, ", ");
            const char* pname = method->params[p].param_name;
            char json_path[128];
            snprintf(json_path, sizeof(json_path), "$.params.%s", pname);
            char* val_str = mg_json_get_str(hm->body, json_path);
            if (!val_str) {
                snprintf(json_path, sizeof(json_path), "$.actions[0].params.params.%s", pname);
                val_str = mg_json_get_str(hm->body, json_path);
            }
            if (!val_str) {
                double num = 0;
                snprintf(json_path, sizeof(json_path), "$.params.%s", pname);
                bool is_num = mg_json_get_num(hm->body, json_path, &num);
                if (!is_num) {
                    snprintf(json_path, sizeof(json_path), "$.actions[0].params.params.%s", pname);
                    is_num = mg_json_get_num(hm->body, json_path, &num);
                }
                if (is_num) {
                    char nbuf[32];
                    snprintf(nbuf, sizeof(nbuf), "%lld", (long long)num);
                    jb_append_str(&inv_jb, nbuf);
                } else {
                    jb_append_str(&inv_jb, "null");
                }
            } else {
                jb_append_str(&inv_jb, "'");
                jb_append_str(&inv_jb, val_str);
                jb_append_str(&inv_jb, "'");
                free(val_str);
            }
        }
    }
    jb_append_str(&inv_jb, ");");

    Value res = execute_apex_source(inv_jb.data);
    free(inv_jb.data);

    char* res_json = val_to_json_string(res);

    JsonBuffer resp_jb;
    jb_init(&resp_jb, strlen(res_json) + 128);
    jb_append_str(&resp_jb, "{\"status\": \"SUCCESS\", \"returnValue\": ");
    jb_append_str(&resp_jb, res_json);
    jb_append_str(&resp_jb, "}");

    send_json_response(c, 200, "OK", resp_jb.data);

    free(res_json);
    free(resp_jb.data);
}

// -----------------------------------------------------------------------------
// Lightning Data Service (UI-API Record Mock) Handlers
// -----------------------------------------------------------------------------

static SObject* find_record_by_id(const char* record_id, const char** out_table_name) {
    if (!record_id) return NULL;
    MockDB* db = mock_db_get_instance();
    if (!db) return NULL;
    for (int t = 0; t < db->table_count; t++) {
        DBTable* tbl = &db->tables[t];
        for (int r = 0; r < tbl->record_count; r++) {
            SObject* rec = tbl->records[r];
            if (!rec || rec->is_deleted) continue;
            Value id_val = sobject_get(rec, "Id");
            if (id_val.type == VAL_STRING && id_val.as.string_val && strcmp(id_val.as.string_val, record_id) == 0) {
                if (out_table_name) *out_table_name = tbl->object_name;
                return rec;
            }
        }
    }
    return NULL;
}

static void handle_ui_api_record_get(struct mg_connection* c, struct mg_http_message* hm, const char* record_id) {
    (void)hm;
    if (!record_id || strlen(record_id) < 3) {
        send_json_response(c, 400, "Bad Request", "{\"message\": \"Invalid recordId\"}");
        return;
    }

    const char* obj_type = NULL;
    SObject* rec = find_record_by_id(record_id, &obj_type);

    if (!rec) {
        obj_type = sobject_type_from_id(record_id);
        // Return simulated UI-API record if not yet inserted
        JsonBuffer jb;
        jb_init(&jb, 512);
        jb_append_str(&jb, "{\n");
        jb_append_str(&jb, "  \"apiName\": \""); jb_append_str(&jb, obj_type); jb_append_str(&jb, "\",\n");
        jb_append_str(&jb, "  \"childRelationships\": {},\n");
        jb_append_str(&jb, "  \"fields\": {\n");
        jb_append_str(&jb, "    \"Id\": { \"displayValue\": null, \"value\": \""); jb_append_str(&jb, record_id); jb_append_str(&jb, "\" },\n");
        jb_append_str(&jb, "    \"Name\": { \"displayValue\": null, \"value\": \"Sample Record\" }\n");
        jb_append_str(&jb, "  },\n");
        jb_append_str(&jb, "  \"id\": \""); jb_append_str(&jb, record_id); jb_append_str(&jb, "\",\n");
        jb_append_str(&jb, "  \"lastModifiedById\": \"005000000000001AAA\",\n");
        jb_append_str(&jb, "  \"lastModifiedDate\": \"2026-09-11T20:00:00.000Z\",\n");
        jb_append_str(&jb, "  \"recordTypeInfo\": null,\n");
        jb_append_str(&jb, "  \"systemModstamp\": \"2026-09-11T20:00:00.000Z\"\n");
        jb_append_str(&jb, "}");
        send_json_response(c, 200, "OK", jb.data);
        free(jb.data);
        return;
    }

    if (!obj_type && rec->type_name) obj_type = rec->type_name;
    if (!obj_type) obj_type = "SObject";

    JsonBuffer jb;
    jb_init(&jb, 1024);
    jb_append_str(&jb, "{\n");
    jb_append_str(&jb, "  \"apiName\": \""); jb_append_str(&jb, obj_type); jb_append_str(&jb, "\",\n");
    jb_append_str(&jb, "  \"childRelationships\": {},\n");
    jb_append_str(&jb, "  \"fields\": {\n");

    bool first = true;
    for (int i = 0; i < rec->field_count; i++) {
        if (!rec->fields[i].name) continue;
        if (!first) jb_append_str(&jb, ",\n");
        first = false;
        jb_append_str(&jb, "    \""); jb_append_str(&jb, rec->fields[i].name); jb_append_str(&jb, "\": { \"displayValue\": null, \"value\": ");
        val_to_json_rec(rec->fields[i].value, &jb);
        jb_append_str(&jb, " }");
    }

    jb_append_str(&jb, "\n  },\n");
    jb_append_str(&jb, "  \"id\": \""); jb_append_str(&jb, record_id); jb_append_str(&jb, "\",\n");
    jb_append_str(&jb, "  \"lastModifiedById\": \"005000000000001AAA\",\n");
    jb_append_str(&jb, "  \"lastModifiedDate\": \"2026-09-11T20:00:00.000Z\",\n");
    jb_append_str(&jb, "  \"recordTypeInfo\": null,\n");
    jb_append_str(&jb, "  \"systemModstamp\": \"2026-09-11T20:00:00.000Z\"\n");
    jb_append_str(&jb, "}");

    send_json_response(c, 200, "OK", jb.data);
    free(jb.data);
}

static void handle_ui_api_object_info(struct mg_connection* c, struct mg_http_message* hm, const char* object_name) {
    (void)hm;
    if (!object_name || object_name[0] == '\0') object_name = "Account";
    const char* pfx = get_sfdc_prefix(object_name);

    JsonBuffer jb;
    jb_init(&jb, 1024);
    jb_append_str(&jb, "{\n");
    jb_append_str(&jb, "  \"apiName\": \""); jb_append_str(&jb, object_name); jb_append_str(&jb, "\",\n");
    jb_append_str(&jb, "  \"label\": \""); jb_append_str(&jb, object_name); jb_append_str(&jb, "\",\n");
    jb_append_str(&jb, "  \"keyPrefix\": \""); jb_append_str(&jb, pfx ? pfx : "001"); jb_append_str(&jb, "\",\n");
    jb_append_str(&jb, "  \"custom\": false,\n");
    jb_append_str(&jb, "  \"queryable\": true,\n");
    jb_append_str(&jb, "  \"createable\": true,\n");
    jb_append_str(&jb, "  \"updateable\": true,\n");
    jb_append_str(&jb, "  \"deletable\": true,\n");
    jb_append_str(&jb, "  \"fields\": {\n");
    jb_append_str(&jb, "    \"Id\": { \"apiName\": \"Id\", \"label\": \"Record ID\", \"dataType\": \"Id\" },\n");
    jb_append_str(&jb, "    \"Name\": { \"apiName\": \"Name\", \"label\": \"Name\", \"dataType\": \"String\" },\n");
    jb_append_str(&jb, "    \"CreatedDate\": { \"apiName\": \"CreatedDate\", \"label\": \"Created Date\", \"dataType\": \"DateTime\" }\n");
    jb_append_str(&jb, "  }\n");
    jb_append_str(&jb, "}");

    send_json_response(c, 200, "OK", jb.data);
    free(jb.data);
}

static void handle_ui_api_create_record(struct mg_connection* c, struct mg_http_message* hm) {
    char* api_name = mg_json_get_str(hm->body, "$.apiName");
    if (!api_name) api_name = mg_json_get_str(hm->body, "$.recordTypeId");
    if (!api_name) api_name = strdup("Account");

    char* name_val = mg_json_get_str(hm->body, "$.fields.Name");
    if (!name_val) name_val = strdup("New Record");

    char apex[512];
    snprintf(apex, sizeof(apex), "%s r = new %s(Name = '%s'); insert r; return r;",
             api_name, api_name, name_val);

    Value res = execute_apex_source(apex);
    if (res.type == VAL_SOBJECT && res.as.sobject_val) {
        Value id_val = sobject_get(res.as.sobject_val, "Id");
        if (id_val.type == VAL_STRING && id_val.as.string_val) {
            handle_ui_api_record_get(c, hm, id_val.as.string_val);
            if (api_name) free(api_name);
            if (name_val) free(name_val);
            return;
        }
    }

    send_json_response(c, 201, "Created", "{\"id\": \"001000000000001AAA\", \"success\": true}");
    if (api_name) free(api_name);
    if (name_val) free(name_val);
}

// -----------------------------------------------------------------------------
// SOQL Query & Tooling API Execute Handler
// -----------------------------------------------------------------------------

static void handle_soql_query(struct mg_connection* c, struct mg_http_message* hm) {
    char qbuf[1024] = {0};
    mg_http_get_var(&hm->query, "q", qbuf, sizeof(qbuf));
    if (qbuf[0] == '\0') {
        send_json_response(c, 400, "Bad Request", "{\"message\": \"Missing q parameter\"}");
        return;
    }

    char final_soql[1200];
    if (qbuf[0] != '[') {
        snprintf(final_soql, sizeof(final_soql), "[%s]", qbuf);
    } else {
        strncpy(final_soql, qbuf, sizeof(final_soql) - 1);
        final_soql[sizeof(final_soql) - 1] = '\0';
    }

    Value res = execute_apex_source(final_soql);
    char* res_json = val_to_json_string(res);

    JsonBuffer jb;
    jb_init(&jb, strlen(res_json) + 128);
    int total = (res.type == VAL_LIST && res.as.list_val) ? res.as.list_val->count : 0;
    char head[64];
    snprintf(head, sizeof(head), "{\"totalSize\": %d, \"done\": true, \"records\": ", total);
    jb_append_str(&jb, head);
    jb_append_str(&jb, res_json);
    jb_append_str(&jb, "}");

    send_json_response(c, 200, "OK", jb.data);

    free(res_json);
    free(jb.data);
}

static void handle_sobject_global_describe(struct mg_connection* c) {
    const char* global_json =
        "{\n"
        "  \"encoding\": \"UTF-8\",\n"
        "  \"maxBatchSize\": 200,\n"
        "  \"sobjects\": [\n"
        "    { \"name\": \"Account\", \"label\": \"Account\", \"keyPrefix\": \"001\", \"custom\": false, \"createable\": true, \"queryable\": true, \"deletable\": true },\n"
        "    { \"name\": \"Contact\", \"label\": \"Contact\", \"keyPrefix\": \"003\", \"custom\": false, \"createable\": true, \"queryable\": true, \"deletable\": true },\n"
        "    { \"name\": \"Opportunity\", \"label\": \"Opportunity\", \"keyPrefix\": \"006\", \"custom\": false, \"createable\": true, \"queryable\": true, \"deletable\": true },\n"
        "    { \"name\": \"Lead\", \"label\": \"Lead\", \"keyPrefix\": \"00Q\", \"custom\": false, \"createable\": true, \"queryable\": true, \"deletable\": true },\n"
        "    { \"name\": \"Case\", \"label\": \"Case\", \"keyPrefix\": \"500\", \"custom\": false, \"createable\": true, \"queryable\": true, \"deletable\": true }\n"
        "  ]\n"
        "}";
    send_json_response(c, 200, "OK", global_json);
}

static void handle_sobject_rest_get(struct mg_connection* c, const char* object_name, const char* record_id) {
    (void)object_name;
    const char* tbl_name = NULL;
    SObject* rec = find_record_by_id(record_id, &tbl_name);
    if (!rec) {
        send_json_response(c, 404, "Not Found", "[{\"message\": \"Provided external ID field does not exist or not found\", \"errorCode\": \"NOT_FOUND\"}]");
        return;
    }
    char* json_str = val_to_json_string(val_sobject(rec));
    send_json_response(c, 200, "OK", json_str);
    free(json_str);
}

static void handle_sobject_rest_create(struct mg_connection* c, struct mg_http_message* hm, const char* object_name) {
    char* name_val = mg_json_get_str(hm->body, "$.Name");
    char* last_name = mg_json_get_str(hm->body, "$.LastName");
    if (!last_name && name_val) last_name = strdup(name_val);
    if (!name_val && !last_name) name_val = strdup("New Record");

    char apex[512];
    if (strcmp(object_name, "Contact") == 0 || strcmp(object_name, "Lead") == 0) {
        snprintf(apex, sizeof(apex), "%s r = new %s(LastName = '%s'); insert r; return r.Id;",
                 object_name, object_name, last_name ? last_name : "Test");
    } else {
        snprintf(apex, sizeof(apex), "%s r = new %s(Name = '%s'); insert r; return r.Id;",
                 object_name, object_name, name_val ? name_val : "Test");
    }
    Value res = execute_apex_source(apex);
    const char* new_id = (res.type == VAL_STRING && res.as.string_val) ? res.as.string_val : "001000000000001AAA";

    char resp[256];
    snprintf(resp, sizeof(resp), "{\"id\": \"%s\", \"success\": true, \"errors\": []}", new_id);
    send_json_response(c, 201, "Created", resp);
    if (name_val) free(name_val);
    if (last_name) free(last_name);
}

static void handle_apex_execute(struct mg_connection* c, struct mg_http_message* hm) {
    char* code = mg_json_get_str(hm->body, "$.code");
    if (!code) code = mg_json_get_str(hm->body, "$.anonymousBody");
    char q_code[2048] = {0};
    if (!code && hm->query.len > 0) {
        mg_http_get_var(&hm->query, "anonymousBody", q_code, sizeof(q_code));
        if (q_code[0] == '\0') {
            mg_http_get_var(&hm->query, "code", q_code, sizeof(q_code));
        }
    }
    const char* src = code ? code : (q_code[0] ? q_code : hm->body.buf);
    size_t src_len = code ? strlen(code) : (q_code[0] ? strlen(q_code) : hm->body.len);

    char* null_term_src = (char*)malloc(src_len + 1);
    if (!null_term_src) abort();
    memcpy(null_term_src, src, src_len);
    null_term_src[src_len] = '\0';

    Value res = execute_apex_source(null_term_src);
    char* res_json = val_to_json_string(res);

    JsonBuffer jb;
    jb_init(&jb, strlen(res_json) + 256);
    jb_append_str(&jb, "{\n"
                       "  \"compiled\": true,\n"
                       "  \"compileProblem\": null,\n"
                       "  \"success\": true,\n"
                       "  \"line\": -1,\n"
                       "  \"column\": -1,\n"
                       "  \"exceptionMessage\": null,\n"
                       "  \"exceptionStackTrace\": null,\n"
                       "  \"result\": ");
    jb_append_str(&jb, res_json);
    jb_append_str(&jb, "\n}");

    send_json_response(c, 200, "OK", jb.data);

    if (code) free(code);
    free(null_term_src);
    free(res_json);
    free(jb.data);
}

// -----------------------------------------------------------------------------
// Standard Salesforce REST Endpoints (Limits, UserInfo, Versions)
// -----------------------------------------------------------------------------

static void handle_limits(struct mg_connection* c) {
    const char* limits_json =
        "{\n"
        "  \"DailyApiRequests\": { \"Max\": 100000, \"Remaining\": 99990 },\n"
        "  \"DataStorageMB\": { \"Max\": 5120, \"Remaining\": 5110 },\n"
        "  \"FileStorageMB\": { \"Max\": 20480, \"Remaining\": 20480 },\n"
        "  \"DailyAsyncApexExecutions\": { \"Max\": 250000, \"Remaining\": 250000 },\n"
        "  \"SingleEmail\": { \"Max\": 5000, \"Remaining\": 5000 }\n"
        "}";
    send_json_response(c, 200, "OK", limits_json);
}

static void handle_versions(struct mg_connection* c) {
    const char* versions_json =
        "[\n"
        "  { \"label\": \"Winter '24\", \"url\": \"/services/data/v58.0\", \"version\": \"58.0\" },\n"
        "  { \"label\": \"Spring '24\", \"url\": \"/services/data/v59.0\", \"version\": \"59.0\" },\n"
        "  { \"label\": \"Summer '24\", \"url\": \"/services/data/v60.0\", \"version\": \"60.0\" }\n"
        "]";
    send_json_response(c, 200, "OK", versions_json);
}

static void handle_userinfo(struct mg_connection* c) {
    const char* user_json =
        "{\n"
        "  \"user_id\": \"005000000000001AAA\",\n"
        "  \"organization_id\": \"00D000000000001AAA\",\n"
        "  \"preferred_username\": \"admin@local.test\",\n"
        "  \"nickname\": \"admin\",\n"
        "  \"name\": \"Nadir Local Administrator\",\n"
        "  \"email\": \"admin@local.test\",\n"
        "  \"zoneinfo\": \"America/Los_Angeles\",\n"
        "  \"locale\": \"en_US\"\n"
        "}";
    send_json_response(c, 200, "OK", user_json);
}

// -----------------------------------------------------------------------------
// Main Event Handler
// -----------------------------------------------------------------------------

static void daemon_http_event_handler(struct mg_connection* c, int ev, void* ev_data) {
    if (ev != MG_EV_HTTP_MSG) return;

    struct mg_http_message* hm = (struct mg_http_message*)ev_data;

    // Handle CORS preflight for all endpoints
    if (mg_strcmp(hm->method, mg_str("OPTIONS")) == 0) {
        send_cors_options(c);
        return;
    }

    // Route: Status / Health
    if (mg_match(hm->uri, mg_str("/"), NULL) || mg_match(hm->uri, mg_str("/status"), NULL) || mg_match(hm->uri, mg_str("/health"), NULL)) {
        char status_json[256];
        snprintf(status_json, sizeof(status_json),
                 "{\"status\": \"ONLINE\", \"server\": \"Nadir Native C11 API Daemon\", \"version\": \"1.0.0\", \"port\": %d}",
                 g_daemon_port);
        send_json_response(c, 200, "OK", status_json);
        return;
    }

    // Route: @AuraEnabled Gateway (/aura or /webruntime/api)
    if (mg_match(hm->uri, mg_str("/aura"), NULL) || mg_match(hm->uri, mg_str("/webruntime/api"), NULL) || mg_match(hm->uri, mg_str("/api/aura"), NULL)) {
        handle_aura_gateway(c, hm);
        return;
    }

    // Route: Lightning Data Service - Object Info
    // Match /services/data/v*/ui-api/object-info/*
    if (mg_match(hm->uri, mg_str("/services/data/*/ui-api/object-info/*"), NULL)) {
        const char* pfx = "/object-info/";
        const char* pos = NULL;
        for (size_t i = 0; i + strlen(pfx) <= hm->uri.len; i++) {
            if (strncmp(hm->uri.buf + i, pfx, strlen(pfx)) == 0) {
                pos = hm->uri.buf + i + strlen(pfx);
                break;
            }
        }
        if (pos) {
            char obj_name[64] = {0};
            size_t idx = 0;
            const char* uri_end = hm->uri.buf + hm->uri.len;
            while (pos < uri_end && *pos != '/' && *pos != '?' && idx < sizeof(obj_name) - 1) {
                obj_name[idx++] = *pos++;
            }
            obj_name[idx] = '\0';
            handle_ui_api_object_info(c, hm, obj_name);
            return;
        }
    }

    // Route: Lightning Data Service - Create Record (POST /services/data/v*/ui-api/records)
    if (mg_match(hm->uri, mg_str("/services/data/*/ui-api/records"), NULL) && mg_strcmp(hm->method, mg_str("POST")) == 0) {
        handle_ui_api_create_record(c, hm);
        return;
    }

    // Route: Lightning Data Service - Single Record (GET /services/data/v*/ui-api/records/*)
    if (mg_match(hm->uri, mg_str("/services/data/*/ui-api/records/*"), NULL)) {
        const char* prefix = "/records/";
        const char* pos = NULL;
        for (size_t i = 0; i + strlen(prefix) <= hm->uri.len; i++) {
            if (strncmp(hm->uri.buf + i, prefix, strlen(prefix)) == 0) {
                pos = hm->uri.buf + i + strlen(prefix);
                break;
            }
        }
        if (pos) {
            char rec_id[64] = {0};
            size_t idx = 0;
            const char* uri_end = hm->uri.buf + hm->uri.len;
            while (pos < uri_end && *pos != '/' && *pos != '?' && idx < sizeof(rec_id) - 1) {
                rec_id[idx++] = *pos++;
            }
            rec_id[idx] = '\0';

            if (mg_strcmp(hm->method, mg_str("DELETE")) == 0) {
                send_json_response(c, 204, "No Content", "");
                return;
            }

            handle_ui_api_record_get(c, hm, rec_id);
            return;
        }
    }

    // Route: Standard SObjects Describe Global (/services/data/v*/sobjects or /services/data/v*/sobjects/)
    if (mg_match(hm->uri, mg_str("/services/data/*/sobjects"), NULL) || mg_match(hm->uri, mg_str("/services/data/*/sobjects/"), NULL)) {
        handle_sobject_global_describe(c);
        return;
    }

    // Route: Standard SObjects REST API (/services/data/v*/sobjects/* or /services/data/v*/sobjects/*/*)
    if (mg_match(hm->uri, mg_str("/services/data/*/sobjects/#"), NULL)) {
        const char* prefix = "/sobjects/";
        const char* pos = NULL;
        for (size_t i = 0; i + strlen(prefix) <= hm->uri.len; i++) {
            if (strncmp(hm->uri.buf + i, prefix, strlen(prefix)) == 0) {
                pos = hm->uri.buf + i + strlen(prefix);
                break;
            }
        }
        if (pos) {
            char segment1[64] = {0};
            char segment2[64] = {0};
            size_t idx = 0;
            const char* uri_end = hm->uri.buf + hm->uri.len;
            while (pos < uri_end && *pos != '/' && *pos != '?' && idx < sizeof(segment1) - 1) {
                segment1[idx++] = *pos++;
            }
            segment1[idx] = '\0';

            if (pos < uri_end && *pos == '/') {
                pos++;
                idx = 0;
                while (pos < uri_end && *pos != '/' && *pos != '?' && idx < sizeof(segment2) - 1) {
                    segment2[idx++] = *pos++;
                }
                segment2[idx] = '\0';
            }

            if (strcmp(segment2, "describe") == 0) {
                handle_ui_api_object_info(c, hm, segment1);
                return;
            }

            if (segment2[0] != '\0') {
                // SObject single record operations
                if (mg_strcmp(hm->method, mg_str("GET")) == 0) {
                    handle_sobject_rest_get(c, segment1, segment2);
                    return;
                } else if (mg_strcmp(hm->method, mg_str("DELETE")) == 0) {
                    send_json_response(c, 204, "No Content", "");
                    return;
                } else if (mg_strcmp(hm->method, mg_str("PATCH")) == 0) {
                    send_json_response(c, 204, "No Content", "");
                    return;
                }
            } else {
                // SObject collection operations
                if (mg_strcmp(hm->method, mg_str("POST")) == 0) {
                    handle_sobject_rest_create(c, hm, segment1);
                    return;
                }
            }
        }
    }

    // Route: SOQL Query REST API
    if (mg_match(hm->uri, mg_str("/services/data/*/query"), NULL) || mg_match(hm->uri, mg_str("/query"), NULL)) {
        handle_soql_query(c, hm);
        return;
    }

    // Route: Salesforce Limits API
    if (mg_match(hm->uri, mg_str("/services/data/*/limits"), NULL)) {
        handle_limits(c);
        return;
    }

    // Route: Salesforce API Versions
    if (mg_match(hm->uri, mg_str("/services/data"), NULL) || mg_match(hm->uri, mg_str("/services/data/"), NULL)) {
        handle_versions(c);
        return;
    }

    // Route: UserInfo Endpoint
    if (mg_match(hm->uri, mg_str("/services/oauth2/userinfo"), NULL)) {
        handle_userinfo(c);
        return;
    }

    // Route: Tooling Anonymous Apex Execute
    if (mg_match(hm->uri, mg_str("/services/data/*/tooling/executeAnonymous"), NULL) || mg_match(hm->uri, mg_str("/aura/execute"), NULL)) {
        handle_apex_execute(c, hm);
        return;
    }

    // Fallback: 404
    send_json_response(c, 404, "Not Found", "{\"message\": \"Resource not found on Nadir API Daemon\"}");
}

int nadir_daemon_start(int port, const char* project_dir, Interpreter* interp) {
    if (port <= 0) port = 8088;
    g_daemon_interp = interp ? interp : interpreter_new();

    if (project_dir) {
        nadir_project_init(project_dir, g_daemon_interp);
    } else {
        nadir_project_init(".", g_daemon_interp);
    }

    struct mg_mgr mgr;
    mg_mgr_init(&mgr);

    int candidate_ports[] = {port, 8088, 8090, 8484, 8585, 9090};
    int num_candidates = (int)(sizeof(candidate_ports) / sizeof(candidate_ports[0]));
    struct mg_connection* conn = NULL;
    char listen_url[64];
    int bound_port = port;

    for (int i = 0; i < num_candidates; i++) {
        bound_port = candidate_ports[i];
        snprintf(listen_url, sizeof(listen_url), "http://0.0.0.0:%d", bound_port);
        conn = mg_http_listen(&mgr, listen_url, daemon_http_event_handler, NULL);
        if (conn) break;
    }

    if (!conn) {
        fprintf(stderr, "FATAL: Failed to bind Nadir API Daemon to %s\n", listen_url);
        mg_mgr_free(&mgr);
        return 1;
    }

    g_daemon_port = bound_port;

    printf("====================================================================\n");
    printf("  NADIR NATIVE C11 API DAEMON RUNNING\n");
    printf("  Listening on: %s\n", listen_url);
    printf("  CORS Target:  * (Accepting Node.js/Vite/LWR Frontend on port 3000/3333)\n");
    printf("  Endpoints:\n");
    printf("    POST /aura                     (@AuraEnabled Gateway)\n");
    printf("    GET  /services/data/v58.0/ui-api/records/{id}  (UI-API Record)\n");
    printf("    GET  /services/data/v58.0/ui-api/object-info/{obj} (UI-API Object Info)\n");
    printf("    POST /services/data/v58.0/ui-api/records (UI-API Create Record)\n");
    printf("    GET  /services/data/v58.0/query (SOQL REST API)\n");
    printf("    GET  /services/data/v58.0/limits (Limits API)\n");
    printf("    GET  /services/oauth2/userinfo (UserInfo API)\n");
    printf("    POST /aura/execute             (Anonymous Apex)\n");
    printf("====================================================================\n");

    while (g_daemon_running) {
        mg_mgr_poll(&mgr, 50);
    }

    mg_mgr_free(&mgr);
    return 0;
}

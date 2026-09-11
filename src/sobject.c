// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#include "sobject.h"
#include <time.h>

SObject* sobject_new(const char* type_name) {
    SObject* o = (SObject*)malloc(sizeof(SObject));
    if (!o) {
        fprintf(stderr, "FATAL: alloc sobject failed for '%s'\n", type_name ? type_name : "null");
        abort();
    }
    o->type_name = nadr_strdup(type_name);
    o->fields = NULL;
    o->field_count = 0;
    o->field_capacity = 0;
    o->is_deleted = false;
    return o;
}

void sobject_put(SObject* obj, const char* field_name, Value val) {
    if (!obj || !field_name) return;

    // apex field names are case insensitive
    for (int i = 0; i < obj->field_count; i++) {
        if (nadr_str_eq(obj->fields[i].name, field_name)) {
            obj->fields[i].value = val;
            return;
        }
    }

    if (obj->field_count + 1 > obj->field_capacity) {
        obj->field_capacity = obj->field_capacity < 8 ? 8 : obj->field_capacity * 2;
        obj->fields = (SObjectField*)realloc(obj->fields, sizeof(SObjectField) * obj->field_capacity);
    }
    obj->fields[obj->field_count].name = nadr_strdup(field_name);
    obj->fields[obj->field_count].value = val;
    obj->field_count++;
}

Value sobject_get(SObject* obj, const char* field_name) {
    if (!obj || !field_name) return val_null();
    for (int k = 0; k < obj->field_count; k++) {
        if (nadr_str_eq(obj->fields[k].name, field_name)) {
            return obj->fields[k].value;
        }
    }
    return val_null();
}

SObject* sobject_clone(SObject* src) {
    if (!src) return NULL;
    SObject* c = sobject_new(src->type_name);
    c->is_deleted = src->is_deleted;
    for (int i = 0; i < src->field_count; i++) {
        sobject_put(c, src->fields[i].name, src->fields[i].value);
    }
    return c;
}

void sobject_free(SObject* obj) {
    if (!obj) return;
    if (obj->type_name) free(obj->type_name);
    for (int f = 0; f < obj->field_count; f++) {
        if (obj->fields[f].name) free(obj->fields[f].name);
    }
    if (obj->fields) free(obj->fields);
    free(obj);
}

// mock in-memory db table storage
static MockDB g_mock_db = {NULL, 0, 0, NULL, 0, 0, 1001};

MockDB* mock_db_get_instance(void) {
    return &g_mock_db;
}

static DBTable* get_or_create_table(MockDB* db, const char* object_name) {
    for (int i = 0; i < db->table_count; i++) {
        if (nadr_str_eq(db->tables[i].object_name, object_name)) {
            return &db->tables[i];
        }
    }
    if (db->table_count + 1 > db->table_capacity) {
        db->table_capacity = db->table_capacity < 8 ? 8 : db->table_capacity * 2;
        db->tables = (DBTable*)realloc(db->tables, sizeof(DBTable) * db->table_capacity);
    }
    DBTable* table = &db->tables[db->table_count++];
    table->object_name = nadr_strdup(object_name);
    table->records = NULL;
    table->record_count = 0;
    table->record_capacity = 0;
    table->auto_id_seq = 0;
    return table;
}

// 3-char standard sfdc prefixes
static const char* get_sfdc_prefix(const char* obj_name) {
    if (nadr_str_eq(obj_name, "Account"))      return "001";
    if (nadr_str_eq(obj_name, "Contact"))      return "003";
    if (nadr_str_eq(obj_name, "Opportunity"))  return "006";
    if (nadr_str_eq(obj_name, "Lead"))         return "00Q";
    if (nadr_str_eq(obj_name, "Task"))         return "00T";
    if (nadr_str_eq(obj_name, "Event"))        return "00U";
    if (nadr_str_eq(obj_name, "Case"))         return "500";
    if (nadr_str_eq(obj_name, "User"))         return "005";
    if (nadr_str_eq(obj_name, "Order"))        return "801";
    if (nadr_str_eq(obj_name, "OrderItem"))    return "802";
    if (nadr_str_eq(obj_name, "Product2"))     return "01t";
    if (nadr_str_eq(obj_name, "Pricebook2"))   return "01s";
    if (nadr_str_eq(obj_name, "Campaign"))     return "701";
    if (nadr_str_eq(obj_name, "Document"))     return "015";
    if (nadr_str_eq(obj_name, "Attachment"))   return "00P";
    return "a00"; // custom objects default
}

// 15 to 18 char id checksum
static void compute_sfdc_checksum(const char* id15, char* out_suffix) {
    static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";
    for (int chunk = 0; chunk < 3; chunk++) {
        int mask = 0;
        for (int i = 0; i < 5; i++) {
            char c = id15[chunk * 5 + i];
            if (c >= 'A' && c <= 'Z') {
                mask |= (1 << i);
            }
        }
        out_suffix[chunk] = tbl[mask];
    }
    out_suffix[3] = '\0';
}

// required fields check
static bool validate_sfdc_schema_rules(SObject* obj, char* err_buf, size_t err_sz) {
    if (!obj) return false;
    
    // required field check
    if (nadr_str_eq(obj->type_name, "Account")) {
        Value name_val = sobject_get(obj, "Name");
        if (name_val.type == VAL_NULL || (name_val.type == VAL_STRING && strlen(name_val.as.string_val) == 0)) {
            snprintf(err_buf, err_sz, "DmlException: Required fields are missing: [Name]");
            return false;
        }
    } else if (nadr_str_eq(obj->type_name, "Contact")) {
        Value last_name = sobject_get(obj, "LastName");
        if (last_name.type == VAL_NULL) {
            snprintf(err_buf, err_sz, "DmlException: Required fields are missing: [LastName]");
            return false;
        }
    }
    return true;
}

Value mock_db_insert(SObject* obj) {
    if (!obj) return val_null();

    char err[256];
    if (!validate_sfdc_schema_rules(obj, err, sizeof(err))) {
        fprintf(stderr, "%s\n", err);
        return val_null();
    }

    MockDB* db = mock_db_get_instance();
    DBTable* table = get_or_create_table(db, obj->type_name);

    table->auto_id_seq++;
    char id15[16];
    char id_full[32];
    const char* pfx = get_sfdc_prefix(obj->type_name);

    // 15-char base id
    snprintf(id15, sizeof(id15), "%s%012d", pfx, table->auto_id_seq);

    // 3-char case safe suffix
    char sfx[4];
    compute_sfdc_checksum(id15, sfx);
    snprintf(id_full, sizeof(id_full), "%s%s", id15, sfx);

    sobject_put(obj, "Id", val_string(id_full));

    // system timestamps
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char date_buf[64];
    strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M:%S", tm_info);
    sobject_put(obj, "CreatedDate", val_string(date_buf));
    sobject_put(obj, "LastModifiedDate", val_string(date_buf));
    sobject_put(obj, "IsDeleted", val_bool(false));

    if (table->record_count + 1 > table->record_capacity) {
        table->record_capacity = table->record_capacity < 8 ? 8 : table->record_capacity * 2;
        table->records = (SObject**)realloc(table->records, sizeof(SObject*) * table->record_capacity);
    }
    table->records[table->record_count++] = obj;
    return val_sobject(obj);
}

Value mock_db_update(SObject* obj) {
    if (!obj) return val_null();
    Value id_val = sobject_get(obj, "Id");
    if (id_val.type != VAL_STRING) {
        fprintf(stderr, "DmlException: Cannot update record without valid 18-char Id\n");
        return val_null();
    }
    MockDB* db = mock_db_get_instance();
    DBTable* table = get_or_create_table(db, obj->type_name);

    for (int i = 0; i < table->record_count; i++) {
        Value cur_id = sobject_get(table->records[i], "Id");
        if (cur_id.type == VAL_STRING && nadr_str_eq(cur_id.as.string_val, id_val.as.string_val)) {
            table->records[i] = obj;
            return val_sobject(obj);
        }
    }
    fprintf(stderr, "DmlException: Record with Id '%s' not found in database\n", id_val.as.string_val);
    return val_null();
}

Value mock_db_delete(SObject* obj) {
    if (!obj) return val_null();
    Value id_val = sobject_get(obj, "Id");
    if (id_val.type != VAL_STRING) return val_null();

    MockDB* db = mock_db_get_instance();
    DBTable* table = get_or_create_table(db, obj->type_name);

    for (int i = 0; i < table->record_count; i++) {
        Value cur_id = sobject_get(table->records[i], "Id");
        if (cur_id.type == VAL_STRING && nadr_str_eq(cur_id.as.string_val, id_val.as.string_val)) {
            table->records[i]->is_deleted = true;
            for (int j = i; j < table->record_count - 1; j++) {
                table->records[j] = table->records[j + 1];
            }
            table->record_count--;
            return val_sobject(obj);
        }
    }
    return val_null();
}

Value mock_db_query(const char* from_obj, const char** fields, int field_count,
                    const char* where_field, const char* where_op, Value where_val, int limit) {
    MockDB* db = mock_db_get_instance();
    DBTable* table = get_or_create_table(db, from_obj);
    Value result_list = val_list();

    for (int i = 0; i < table->record_count; i++) {
        SObject* src = table->records[i];
        if (src->is_deleted) continue; // skip deleted

        bool matches = true;

        if (where_field && where_op) {
            Value actual = sobject_get(src, where_field);
            if (nadr_str_eq(where_op, "=") || nadr_str_eq(where_op, "==")) {
                matches = val_equals(actual, where_val);
            } else if (nadr_str_eq(where_op, "!=") || nadr_str_eq(where_op, "<>")) {
                matches = !val_equals(actual, where_val);
            } else if (nadr_str_eq(where_op, ">")) {
                if (actual.type == VAL_INT && where_val.type == VAL_INT) {
                    matches = actual.as.int_val > where_val.as.int_val;
                } else if ((actual.type == VAL_DOUBLE || actual.type == VAL_INT) &&
                           (where_val.type == VAL_DOUBLE || where_val.type == VAL_INT)) {
                    double d1 = actual.type == VAL_DOUBLE ? actual.as.double_val : (double)actual.as.int_val;
                    double d2 = where_val.type == VAL_DOUBLE ? where_val.as.double_val : (double)where_val.as.int_val;
                    matches = d1 > d2;
                }
            } else if (nadr_str_eq(where_op, "<")) {
                if (actual.type == VAL_INT && where_val.type == VAL_INT) {
                    matches = actual.as.int_val < where_val.as.int_val;
                } else if ((actual.type == VAL_DOUBLE || actual.type == VAL_INT) &&
                           (where_val.type == VAL_DOUBLE || where_val.type == VAL_INT)) {
                    double d1 = actual.type == VAL_DOUBLE ? actual.as.double_val : (double)actual.as.int_val;
                    double d2 = where_val.type == VAL_DOUBLE ? where_val.as.double_val : (double)where_val.as.int_val;
                    matches = d1 < d2;
                }
            } else if (nadr_str_eq(where_op, ">=")) {
                if (actual.type == VAL_INT && where_val.type == VAL_INT) {
                    matches = actual.as.int_val >= where_val.as.int_val;
                }
            } else if (nadr_str_eq(where_op, "<=")) {
                if (actual.type == VAL_INT && where_val.type == VAL_INT) {
                    matches = actual.as.int_val <= where_val.as.int_val;
                }
            }
        }

        if (matches) {
            SObject* record_copy = sobject_new(from_obj);
            // project fields
            for (int f = 0; f < field_count; f++) {
                Value fv = sobject_get(src, fields[f]);
                sobject_put(record_copy, fields[f], fv);
            }
            // always include id
            Value id_val = sobject_get(src, "Id");
            if (id_val.type == VAL_STRING) {
                sobject_put(record_copy, "Id", id_val);
            }
            val_list_add(&result_list, val_sobject(record_copy));

            if (limit > 0 && val_list_size(&result_list) >= limit) {
                break;
            }
        }
    }

    return result_list;
}

// savepoint snapshot
int mock_db_set_savepoint(void) {
    MockDB* db = mock_db_get_instance();
    if (db->savepoint_count + 1 > db->savepoint_capacity) {
        db->savepoint_capacity = db->savepoint_capacity < 4 ? 4 : db->savepoint_capacity * 2;
        db->savepoints = (DBSnapshot*)realloc(db->savepoints, sizeof(DBSnapshot) * db->savepoint_capacity);
    }
    int sp_id = db->next_savepoint_id++;
    DBSnapshot* snap = &db->savepoints[db->savepoint_count++];
    snap->savepoint_id = sp_id;
    snap->saved_table_count = db->table_count;
    snap->saved_tables = (DBTable*)malloc(sizeof(DBTable) * (db->table_count > 0 ? db->table_count : 1));

    for (int t = 0; t < db->table_count; t++) {
        DBTable* src_t = &db->tables[t];
        DBTable* dst_t = &snap->saved_tables[t];
        dst_t->object_name = nadr_strdup(src_t->object_name);
        dst_t->record_count = src_t->record_count;
        dst_t->record_capacity = src_t->record_count;
        dst_t->auto_id_seq = src_t->auto_id_seq;
        dst_t->records = (SObject**)malloc(sizeof(SObject*) * (src_t->record_count > 0 ? src_t->record_count : 1));
        for (int r = 0; r < src_t->record_count; r++) {
            dst_t->records[r] = sobject_clone(src_t->records[r]);
        }
    }
    return sp_id;
}

// rollback to savepoint
void mock_db_rollback(int savepoint_id) {
    MockDB* db = mock_db_get_instance();
    int idx = -1;
    for (int i = db->savepoint_count - 1; i >= 0; i--) {
        if (db->savepoints[i].savepoint_id == savepoint_id) {
            idx = i;
            break;
        }
    }
    if (idx == -1) return;

    DBSnapshot* snap = &db->savepoints[idx];

    // restore table states
    for (int t = 0; t < db->table_count; t++) {
        for (int r = 0; r < db->tables[t].record_count; r++) {
            sobject_free(db->tables[t].records[r]);
        }
        if (db->tables[t].records) free(db->tables[t].records);
        if (db->tables[t].object_name) free(db->tables[t].object_name);
    }
    free(db->tables);

    db->table_count = snap->saved_table_count;
    db->table_capacity = snap->saved_table_count;
    db->tables = (DBTable*)malloc(sizeof(DBTable) * (db->table_count > 0 ? db->table_count : 1));

    for (int t = 0; t < snap->saved_table_count; t++) {
        DBTable* src_t = &snap->saved_tables[t];
        DBTable* dst_t = &db->tables[t];
        dst_t->object_name = nadr_strdup(src_t->object_name);
        dst_t->record_count = src_t->record_count;
        dst_t->record_capacity = src_t->record_count;
        dst_t->auto_id_seq = src_t->auto_id_seq;
        dst_t->records = (SObject**)malloc(sizeof(SObject*) * (src_t->record_count > 0 ? src_t->record_count : 1));
        for (int r = 0; r < src_t->record_count; r++) {
            dst_t->records[r] = sobject_clone(src_t->records[r]);
        }
    }
}

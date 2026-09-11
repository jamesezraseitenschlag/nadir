// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#ifndef NADIR_SOBJECT_H
#define NADIR_SOBJECT_H

#include "common.h"
#include "value.h"

struct Value;

// field on an sobject
typedef struct SObjectField {
    char* name;
    struct Value value;
} SObjectField;

// sobject record
typedef struct SObject {
    char* type_name;
    SObjectField* fields;
    int field_count;
    int field_capacity;
    bool is_deleted;
} SObject;

SObject* sobject_new(const char* type_name);
void sobject_put(SObject* obj, const char* field_name, struct Value val);
struct Value sobject_get(SObject* obj, const char* field_name);
SObject* sobject_clone(SObject* src);
void sobject_free(SObject* obj);

// in-memory table
typedef struct DBTable {
    char* object_name;
    SObject** records;
    int record_count;
    int record_capacity;
    int auto_id_seq;
} DBTable;

// savepoint snapshot
typedef struct DBSnapshot {
    int savepoint_id;
    DBTable* saved_tables;
    int saved_table_count;
} DBSnapshot;

// simple mock database
typedef struct MockDB {
    DBTable* tables;
    int table_count;
    int table_capacity;
    DBSnapshot* savepoints;
    int savepoint_count;
    int savepoint_capacity;
    int next_savepoint_id;
} MockDB;

MockDB* mock_db_get_instance(void);
struct Value mock_db_insert(SObject* obj);
struct Value mock_db_update(SObject* obj);
struct Value mock_db_delete(SObject* obj);
struct Value mock_db_query(const char* from_obj, const char** fields, int field_count,
                           const char* where_field, const char* where_op, struct Value where_val, int limit);

int mock_db_set_savepoint(void);
void mock_db_rollback(int savepoint_id);

#endif


// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#ifndef NADIR_METADATA_H
#define NADIR_METADATA_H

#include "common.h"
#include "value.h"
#include "sobject.h"
#include "eval.h"

typedef struct MetaField {
    char* full_name;
    char* label;
    char* type;
    char* reference_to;
    int length;
    int precision;
    int scale;
    bool required;
    bool unique;
    bool external_id;
    char* default_value;
} MetaField;

typedef struct MetaObject {
    char* full_name;
    char* label;
    char* sharing_model;
    MetaField* fields;
    int field_count;
    int field_capacity;
} MetaObject;

typedef struct ProjectSchema {
    MetaObject* objects;
    int object_count;
    int object_capacity;
    char* project_root;
} ProjectSchema;

ProjectSchema* project_schema_get_instance(void);
void project_schema_reset(void);
MetaObject* project_schema_get_or_create_object(const char* obj_name);
void project_schema_add_field(MetaObject* obj, MetaField field);
MetaObject* project_schema_get_object(const char* obj_name);
MetaField* project_schema_get_field(const char* obj_name, const char* field_name);

int nadir_project_init(const char* root_path, Interpreter* interp);

#endif

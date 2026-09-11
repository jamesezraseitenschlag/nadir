// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#ifndef NADIR_VALUE_H
#define NADIR_VALUE_H

#include "common.h"

typedef enum {
    VAL_NULL,
    VAL_INT,
    VAL_DOUBLE,
    VAL_BOOL,
    VAL_STRING,
    VAL_LIST,
    VAL_MAP,
    VAL_SOBJECT,
    VAL_INSTANCE,
    VAL_NATIVE_FN
} ValueType;

struct Value;
struct SObject;
struct ApexInstance;

typedef struct ValueArray {
    struct Value* items;
    int count;
    int capacity;
} ValueArray;

typedef struct MapEntry {
    struct Value* key;
    struct Value* value;
} MapEntry;

typedef struct ValueMap {
    MapEntry* entries;
    int count;
    int capacity;
} ValueMap;

typedef struct Value (*NativeFn)(int arg_count, struct Value* args);

typedef struct Value {
    ValueType type;
    union {
        int64_t int_val;
        double double_val;
        bool bool_val;
        char* string_val;
        ValueArray* list_val;
        ValueMap* map_val;
        struct SObject* sobject_val;
        struct ApexInstance* instance_val;
        NativeFn native_fn;
    } as;
} Value;

// Value Constructors
Value val_null(void);
Value val_int(int64_t v);
Value val_double(double v);
Value val_bool(bool v);
Value val_string(const char* s);
Value val_string_slice(const char* s, int len);
Value val_list(void);
Value val_map(void);
Value val_sobject(struct SObject* obj);
Value val_instance(struct ApexInstance* inst);
Value val_native_fn(NativeFn fn);

// List / Map helpers
void val_list_add(Value* list_val, Value item);
Value val_list_get(Value* list_val, int index);
void val_list_set(Value* list_val, int index, Value item);
int val_list_size(Value* list_val);

void val_map_put(Value* map_val, Value key, Value val);
Value val_map_get(Value* map_val, Value key);
int val_map_size(Value* map_val);

// Utility
bool val_is_truthy(Value v);
bool val_equals(Value a, Value b);
char* val_to_string(Value v);
void val_print(Value v);
void val_free(Value v);

#endif // VALUE_H

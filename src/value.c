// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#include "value.h"
#include "sobject.h"
#include "nadir_hash.h"

Value val_null(void) {
    Value v;
    v.type = VAL_NULL;
    v.as.int_val = 0;
    return v;
}

Value val_int(int64_t v) {
    Value val;
    val.type = VAL_INT;
    val.as.int_val = v;
    return val;
}

Value val_double(double v) {
    Value val;
    val.type = VAL_DOUBLE;
    val.as.double_val = v;
    return val;
}

Value val_bool(bool v) {
    Value val;
    val.type = VAL_BOOL;
    val.as.bool_val = v;
    return val;
}

Value val_string(const char* s) {
    Value val;
    val.type = VAL_STRING;
    val.as.string_val = duplicate_string(s);
    return val;
}

Value val_string_slice(const char* s, int len) {
    Value val;
    val.type = VAL_STRING;
    val.as.string_val = duplicate_slice(s, len);
    return val;
}

Value val_list(void) {
    Value val;
    val.type = VAL_LIST;
    val.as.list_val = (ValueArray*)malloc(sizeof(ValueArray));
    val.as.list_val->items = NULL;
    val.as.list_val->count = 0;
    val.as.list_val->capacity = 0;
    return val;
}

Value val_sobject(struct SObject* obj) {
    Value val;
    val.type = VAL_SOBJECT;
    val.as.sobject_val = obj;
    return val;
}

Value val_instance(struct ApexInstance* inst) {
    Value val;
    val.type = VAL_INSTANCE;
    val.as.instance_val = inst;
    return val;
}

Value val_native_fn(NativeFn fn) {
    Value val;
    val.type = VAL_NATIVE_FN;
    val.as.native_fn = fn;
    return val;
}

void val_list_add(Value* list_val, Value item) {
    if (list_val->type != VAL_LIST) return;
    ValueArray* arr = list_val->as.list_val;
    if (arr->count + 1 > arr->capacity) {
        arr->capacity = arr->capacity < 8 ? 8 : arr->capacity * 2;
        arr->items = (Value*)realloc(arr->items, sizeof(Value) * arr->capacity);
    }
    arr->items[arr->count++] = item;
}

Value val_list_get(Value* list_val, int index) {
    if (list_val->type != VAL_LIST) return val_null();
    ValueArray* arr = list_val->as.list_val;
    if (index < 0 || index >= arr->count) return val_null();
    return arr->items[index];
}

void val_list_set(Value* list_val, int index, Value item) {
    if (list_val->type != VAL_LIST) return;
    ValueArray* arr = list_val->as.list_val;
    if (index >= 0 && index < arr->count) {
        arr->items[index] = item;
    }
}

int val_list_size(Value* list_val) {
    if (list_val->type != VAL_LIST) return 0;
    return list_val->as.list_val->count;
}

static inline uint64_t compute_val_hash(Value v) {
    switch (v.type) {
        case VAL_INT:
            return (uint64_t)v.as.int_val * 11400714785074694791ULL;
        case VAL_DOUBLE: {
            uint64_t u;
            memcpy(&u, &v.as.double_val, sizeof(uint64_t));
            return u * 11400714785074694791ULL;
        }
        case VAL_BOOL:
            return v.as.bool_val ? 0x9e3779b97f4a7c15ULL : 0x123456789abcdef0ULL;
        case VAL_STRING:
            if (!v.as.string_val) return 0;
            return (uint64_t)hash_string_case_xxhash(v.as.string_val);
        case VAL_SOBJECT:
            return (uint64_t)(uintptr_t)v.as.sobject_val * 11400714785074694791ULL;
        case VAL_INSTANCE:
            return (uint64_t)(uintptr_t)v.as.instance_val * 11400714785074694791ULL;
        default:
            return 0;
    }
}

Value val_map(void) {
    Value val;
    val.type = VAL_MAP;
    val.as.map_val = (ValueMap*)malloc(sizeof(ValueMap));
    val.as.map_val->capacity = 32;
    val.as.map_val->count = 0;
    val.as.map_val->entries = (MapEntry*)calloc(val.as.map_val->capacity, sizeof(MapEntry));
    return val;
}

static void val_map_rehash(ValueMap* map, int new_cap) {
    MapEntry* old_entries = map->entries;
    int old_cap = map->capacity;

    map->entries = (MapEntry*)calloc(new_cap, sizeof(MapEntry));
    map->capacity = new_cap;
    map->count = 0;

    uint64_t mask = (uint64_t)(new_cap - 1);
    for (int i = 0; i < old_cap; i++) {
        if (old_entries[i].occupied) {
            uint64_t h = old_entries[i].hash;
            int idx = (int)(h & mask);
            while (map->entries[idx].occupied) {
                idx = (idx + 1) & (int)mask;
            }
            map->entries[idx].hash = h;
            map->entries[idx].key = old_entries[i].key;
            map->entries[idx].value = old_entries[i].value;
            map->entries[idx].occupied = true;
            map->count++;
        }
    }
    if (old_entries) free(old_entries);
}

void val_map_put(Value* map_val, Value key, Value val) {
    if (map_val->type != VAL_MAP) return;
    ValueMap* map = map_val->as.map_val;
    if (!map) return;

    if (map->capacity == 0 || !map->entries) {
        map->capacity = 32;
        map->entries = (MapEntry*)calloc(map->capacity, sizeof(MapEntry));
    } else if (map->count * 4 >= map->capacity * 3) {
        val_map_rehash(map, map->capacity * 2);
    }

    uint64_t h = compute_val_hash(key);
    uint64_t mask = (uint64_t)(map->capacity - 1);
    int idx = (int)(h & mask);

    while (map->entries[idx].occupied) {
        if (map->entries[idx].hash == h && val_equals(map->entries[idx].key, key)) {
            map->entries[idx].value = val;
            return;
        }
        idx = (idx + 1) & (int)mask;
    }

    map->entries[idx].hash = h;
    map->entries[idx].key = key;
    map->entries[idx].value = val;
    map->entries[idx].occupied = true;
    map->count++;
}

Value val_map_get(Value* map_val, Value key) {
    if (map_val->type != VAL_MAP) return val_null();
    ValueMap* map = map_val->as.map_val;
    if (!map || map->count == 0 || map->capacity == 0 || !map->entries) return val_null();

    uint64_t h = compute_val_hash(key);
    uint64_t mask = (uint64_t)(map->capacity - 1);
    int idx = (int)(h & mask);

    while (map->entries[idx].occupied) {
        if (map->entries[idx].hash == h && val_equals(map->entries[idx].key, key)) {
            return map->entries[idx].value;
        }
        idx = (idx + 1) & (int)mask;
    }
    return val_null();
}

int val_map_size(Value* map_val) {
    if (map_val->type != VAL_MAP || !map_val->as.map_val) return 0;
    return map_val->as.map_val->count;
}

bool val_is_truthy(Value v) {
    switch (v.type) {
        case VAL_NULL: return false;
        case VAL_BOOL: return v.as.bool_val;
        case VAL_INT: return v.as.int_val != 0;
        case VAL_DOUBLE: return v.as.double_val != 0.0;
        case VAL_STRING: return v.as.string_val && strlen(v.as.string_val) > 0;
        default: return true;
    }
}

bool val_equals(Value a, Value b) {
    if (a.type != b.type) return false;
    switch (a.type) {
        case VAL_NULL: return true;
        case VAL_BOOL: return a.as.bool_val == b.as.bool_val;
        case VAL_INT: return a.as.int_val == b.as.int_val;
        case VAL_DOUBLE: return a.as.double_val == b.as.double_val;
        case VAL_STRING: return string_equal_case(a.as.string_val, b.as.string_val);
        case VAL_SOBJECT: return a.as.sobject_val == b.as.sobject_val;
        case VAL_INSTANCE: return a.as.instance_val == b.as.instance_val;
        default: return false;
    }
}

char* val_to_string(Value v) {
    char buf[256];
    switch (v.type) {
        case VAL_NULL: return duplicate_string("null");
        case VAL_BOOL: return duplicate_string(v.as.bool_val ? "true" : "false");
        case VAL_INT:
            snprintf(buf, sizeof(buf), "%lld", (long long)v.as.int_val);
            return duplicate_string(buf);
        case VAL_DOUBLE:
            snprintf(buf, sizeof(buf), "%g", v.as.double_val);
            return duplicate_string(buf);
        case VAL_STRING:
            return duplicate_string(v.as.string_val ? v.as.string_val : "");
        case VAL_LIST: {
            size_t size = 128;
            char* out = (char*)malloc(size);
            strcpy(out, "(");
            for (int i = 0; i < v.as.list_val->count; i++) {
                char* item_s = val_to_string(v.as.list_val->items[i]);
                size_t needed = strlen(out) + strlen(item_s) + 4;
                if (needed > size) {
                    size = needed * 2;
                    out = (char*)realloc(out, size);
                }
                if (i > 0) strcat(out, ", ");
                strcat(out, item_s);
                free(item_s);
            }
            strcat(out, ")");
            return out;
        }
        case VAL_MAP: {
            size_t size = 128;
            char* out = (char*)malloc(size);
            strcpy(out, "{");
            int printed = 0;
            if (v.as.map_val && v.as.map_val->entries) {
                for (int i = 0; i < v.as.map_val->capacity; i++) {
                    if (!v.as.map_val->entries[i].occupied) continue;
                    char* ks = val_to_string(v.as.map_val->entries[i].key);
                    char* vs = val_to_string(v.as.map_val->entries[i].value);
                    size_t needed = strlen(out) + strlen(ks) + strlen(vs) + 8;
                    if (needed > size) {
                        size = needed * 2;
                        out = (char*)realloc(out, size);
                    }
                    if (printed > 0) strcat(out, ", ");
                    strcat(out, ks);
                    strcat(out, " => ");
                    strcat(out, vs);
                    free(ks);
                    free(vs);
                    printed++;
                }
            }
            strcat(out, "}");
            return out;
        }
        case VAL_SOBJECT: {
            if (!v.as.sobject_val) return duplicate_string("null");
            SObject* obj = v.as.sobject_val;
            size_t size = 256;
            char* out = (char*)malloc(size);
            snprintf(out, size, "%s:{", obj->type_name);
            for (int i = 0; i < obj->field_count; i++) {
                char* fs = val_to_string(obj->fields[i].value);
                size_t needed = strlen(out) + strlen(obj->fields[i].name) + strlen(fs) + 6;
                if (needed > size) {
                    size = needed * 2;
                    out = (char*)realloc(out, size);
                }
                if (i > 0) strcat(out, ", ");
                strcat(out, obj->fields[i].name);
                strcat(out, ": ");
                strcat(out, fs);
                free(fs);
            }
            strcat(out, "}");
            return out;
        }
        case VAL_INSTANCE: {
            snprintf(buf, sizeof(buf), "<ApexInstance@%p>", (void*)v.as.instance_val);
            return duplicate_string(buf);
        }
        default:
            return duplicate_string("<Value>");
    }
}

void val_print(Value v) {
    char* s = val_to_string(v);
    printf("%s", s);
    free(s);
}

void val_free(Value v) {
    if (v.type == VAL_STRING && v.as.string_val) {
        free(v.as.string_val);
    }
}

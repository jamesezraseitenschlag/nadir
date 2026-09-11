// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#include "env.h"

Environment* env_new(Environment* parent) {
    Environment* e = (Environment*)malloc(sizeof(Environment));
    e->parent = parent;
    e->entries = NULL;
    e->count = 0;
    e->capacity = 0;
    return e;
}

void env_free(Environment* e) {
    if (!e) return;
    for (int i = 0; i < e->count; i++) {
        if (e->entries[i].name) free(e->entries[i].name);
    }
    if (e->entries) free(e->entries);
    free(e);
}

void env_define(Environment* e, const char* name, Value val) {
    if (!e || !name) return;
    // case insensitive lookup
    for (int i = 0; i < e->count; i++) {
        if (string_equal_case(e->entries[i].name, name)) {
            e->entries[i].value = val;
            return;
        }
    }
    if (e->count + 1 > e->capacity) {
        e->capacity = e->capacity < 8 ? 8 : e->capacity * 2;
        e->entries = (EnvEntry*)realloc(e->entries, sizeof(EnvEntry) * e->capacity);
    }
    e->entries[e->count].name = duplicate_string(name);
    e->entries[e->count].value = val;
    e->count++;
}

bool env_assign(Environment* e, const char* var, Value val) {
    if (!e || !var) return false;
    for (int i = 0; i < e->count; i++) {
        if (string_equal_case(e->entries[i].name, var)) {
            e->entries[i].value = val;
            return true;
        }
    }
    if (e->parent) {
        return env_assign(e->parent, var, val);
    }
    return false;
}

bool env_get(Environment* e, const char* name, Value* out) {
    if (!e || !name) return false;
    for (int i = 0; i < e->count; i++) {
        if (string_equal_case(e->entries[i].name, name)) {
            if (out) *out = e->entries[i].value;
            return true;
        }
    }
    if (e->parent) {
        return env_get(e->parent, name, out);
    }
    return false;
}

bool env_has(Environment* e, const char* name) {
    return env_get(e, name, NULL);
}


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

void env_define_prehashed(Environment* e, const char* name, uint32_t hash, Value val) {
    if (!e || !name) return;
    for (int i = 0; i < e->count; i++) {
        if (e->entries[i].hash == hash && nadr_str_eq(e->entries[i].name, name)) {
            e->entries[i].value = val;
            return;
        }
    }
    if (e->count + 1 > e->capacity) {
        e->capacity = e->capacity < 8 ? 8 : e->capacity * 2;
        e->entries = (EnvEntry*)realloc(e->entries, sizeof(EnvEntry) * (size_t)e->capacity);
        if (!e->entries) abort();
    }
    e->entries[e->count].name = duplicate_string(name);
    e->entries[e->count].hash = hash;
    e->entries[e->count].value = val;
    e->count++;
}

void env_define(Environment* e, const char* name, Value val) {
    if (!name) return;
    env_define_prehashed(e, name, nadr_hash_str(name), val);
}

bool env_assign_prehashed(Environment* e, const char* var, uint32_t hash, Value val) {
    if (!e || !var) return false;
    for (Environment* cur = e; cur; cur = cur->parent) {
        for (int i = 0; i < cur->count; i++) {
            if (cur->entries[i].hash == hash && nadr_str_eq(cur->entries[i].name, var)) {
                cur->entries[i].value = val;
                return true;
            }
        }
    }
    return false;
}

bool env_assign(Environment* e, const char* var, Value val) {
    if (!var) return false;
    return env_assign_prehashed(e, var, nadr_hash_str(var), val);
}

bool env_get_prehashed(Environment* e, const char* name, uint32_t hash, Value* out) {
    if (!e || !name) return false;
    for (Environment* cur = e; cur; cur = cur->parent) {
        for (int i = 0; i < cur->count; i++) {
            if (cur->entries[i].hash == hash && nadr_str_eq(cur->entries[i].name, name)) {
                if (out) *out = cur->entries[i].value;
                return true;
            }
        }
    }
    return false;
}

bool env_get(Environment* e, const char* name, Value* out) {
    if (!name) return false;
    return env_get_prehashed(e, name, nadr_hash_str(name), out);
}

bool env_has(Environment* e, const char* name) {
    return env_get(e, name, NULL);
}


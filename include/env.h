// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#ifndef NADIR_ENV_H
#define NADIR_ENV_H

#include "common.h"
#include "value.h"

typedef struct EnvEntry {
    char* name;
    Value value;
} EnvEntry;

typedef struct Environment {
    struct Environment* parent;
    EnvEntry* entries;
    int count;
    int capacity;
} Environment;

Environment* env_new(Environment* parent);
void env_free(Environment* env);
void env_define(Environment* env, const char* name, Value value);
bool env_assign(Environment* env, const char* name, Value value);
bool env_get(Environment* env, const char* name, Value* out_value);
bool env_has(Environment* env, const char* name);

#endif // ENV_H

// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#ifndef NADIR_COMMON_H
#define NADIR_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#if defined(_WIN32)
  #define strcasecmp _stricmp
  #define strncasecmp _strnicmp
#else
  #include <strings.h>
#endif

#define STRCASECMP strcasecmp
#define STRNCASECMP strncasecmp

// sfdc id stuff
#define SFDC_PREFIX_LEN    3   // 001 account 003 contact etc
#define SFDC_BASE_ID_LEN  15   // old 15 char id
#define SFDC_FULL_ID_LEN  18   // 18 char case safe id

#define NADIR_MAX_CALL_DEPTH 512

// djb2 hash case folded
static inline uint32_t nadr_hash_str(const char* s) {
    uint32_t h = 5381;
    if (!s) return h;
    while (*s) {
        h = ((h << 5) + h) + (uint8_t)tolower((unsigned char)*s);
        s++;
    }
    return h;
}

static inline uint32_t nadr_hash_slice(const char* s, int len) {
    uint32_t h = 5381;
    if (!s || len <= 0) return h;
    for (int i = 0; i < len; i++) {
        h = ((h << 5) + h) + (uint8_t)tolower((unsigned char)s[i]);
    }
    return h;
}

static inline bool nadr_str_eq(const char* a, const char* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    return strcasecmp(a, b) == 0;
}

static inline bool nadr_slice_eq(const char* a, int a_len, const char* b, int b_len) {
    if (a_len != b_len) return false;
    if (a_len == 0) return true;
    return strncasecmp(a, b, a_len) == 0;
}


static inline char* nadr_strdup(const char* s) {
    if (!s) return NULL;
    size_t sz = strlen(s) + 1;
    char* p = (char*)malloc(sz);
    if (!p) {
        fprintf(stderr, "FATAL: out of memory in nadr_strdup (%zu bytes)\n", sz);
        abort();
    }
    memcpy(p, s, sz);
    return p;
}

static inline char* nadr_strndup(const char* s, int len) {
    if (!s || len < 0) return nadr_strdup("");
    char* p = (char*)malloc((size_t)len + 1);
    if (!p) {
        fprintf(stderr, "FATAL: out of memory in nadr_strndup (%d bytes)\n", len + 1);
        abort();
    }
    memcpy(p, s, (size_t)len);
    p[len] = '\0';
    return p;
}

static inline char* nadr_read_file(const char* path) {
    if (!path) return NULL;
    FILE* file = fopen(path, "rb");
    if (!file) return NULL;
    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);
    char* buffer = (char*)malloc(file_size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
    buffer[bytes_read] = '\0';
    fclose(file);
    return buffer;
}

// old aliases, pain
#define string_equal_case nadr_str_eq
#define slice_equal_case nadr_slice_eq
#define duplicate_string nadr_strdup
#define duplicate_slice nadr_strndup
#define hash_string_case nadr_hash_str
#define hash_slice_case nadr_hash_slice
#define read_file nadr_read_file

#endif

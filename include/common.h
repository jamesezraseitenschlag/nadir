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
  #define NADIR_HAS_CLOCK_GETTIME 0
#else
  #include <strings.h>
  #define NADIR_HAS_CLOCK_GETTIME 1
#endif

// Win32 does not ship clock_gettime in older SDKs and <windows.h> would
// collide with our own TokenType enum, so the two QPC/FT entry points we
// need are declared here by hand and resolved from kernel32 at link time.
#if !NADIR_HAS_CLOCK_GETTIME
  #ifndef NADIR_WIN32_TYPES_DECLARED
  #define NADIR_WIN32_TYPES_DECLARED
  typedef union { struct { uint32_t LowPart; int32_t HighPart; }; int64_t QuadPart; } NadirLargeInteger;
  typedef struct { uint32_t dwLowDateTime; uint32_t dwHighDateTime; } NadirFileTime;
  typedef struct { uint32_t dwLowDateTime; uint32_t dwHighDateTime; } NadirSystemTime;
  __declspec(dllimport) int __stdcall QueryPerformanceFrequency(NadirLargeInteger* const freq);
  __declspec(dllimport) int __stdcall QueryPerformanceCounter(NadirLargeInteger* const counter);
  __declspec(dllimport) void __stdcall GetSystemTimeAsFileTime(NadirFileTime* const ft);
  #endif
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
    // Cheap rejections first. Apex field lookup compares a probe name against
    // every field on the record, and most probes differ in length or in the
    // very first character, so strcasecmp almost never has to run.
    if (a[0] != b[0]) {
        unsigned char ca = (unsigned char)a[0];
        unsigned char cb = (unsigned char)b[0];
        if (ca >= 'A' && ca <= 'Z') ca = (unsigned char)(ca + 32);
        if (cb >= 'A' && cb <= 'Z') cb = (unsigned char)(cb + 32);
        if (ca != cb) return false;
    }
    if (a[1] == '\0' || b[1] == '\0') return a[1] == b[1];
    return strcasecmp(a, b) == 0;
}

static inline bool nadr_slice_eq(const char* a, int a_len, const char* b, int b_len) {
    if (a_len != b_len) return false;
    if (a_len == 0) return true;
    return strncasecmp(a, b, a_len) == 0;
}


// monotonic millisecond clock, used for governor timing and benchmarks.
// time(NULL) only has second resolution, which is useless for profiling.
static inline int64_t nadr_monotonic_ms(void) {
#if NADIR_HAS_CLOCK_GETTIME
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0;
    return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#else
    NadirLargeInteger freq, now;
    static NadirLargeInteger cached_freq = {0};
    if (cached_freq.QuadPart == 0) QueryPerformanceFrequency(&cached_freq);
    freq = cached_freq;
    QueryPerformanceCounter(&now);
    if (freq.QuadPart == 0) return 0;
    return (int64_t)((now.QuadPart * 1000) / freq.QuadPart);
#endif
}

// wall clock in milliseconds since the unix epoch (Apex semantics).
// This used to be time(NULL) * 1000, which silently quantised every
// measurement to whole seconds.
static inline int64_t nadir_epoch_ms(void) {
#if NADIR_HAS_CLOCK_GETTIME
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) return 0;
    return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#else
    NadirFileTime ft;
    int64_t ticks;
    GetSystemTimeAsFileTime(&ft);
    ticks = ((int64_t)ft.dwHighDateTime << 32) | (int64_t)ft.dwLowDateTime;
    // 100ns ticks since 1601-01-01 -> ms since 1970-01-01
    return (ticks / 10000LL) - 11644473600000LL;
#endif
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

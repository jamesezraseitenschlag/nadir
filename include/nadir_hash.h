// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#ifndef NADIR_HASH_H
#define NADIR_HASH_H

#include "common.h"

// MurmurHash3 32-bit & 128-bit
uint32_t murmur3_32(const void* key, size_t len, uint32_t seed);
void murmur3_128(const void* key, size_t len, uint32_t seed, void* out);

// xxHash 32-bit & 64-bit
uint32_t xxhash32(const void* input, size_t len, uint32_t seed);
uint64_t xxhash64(const void* input, size_t len, uint64_t seed);

// Case-insensitive string hashing utilities for Apex identifiers and symbols
uint32_t hash_string_case_murmur3(const char* str);
uint32_t hash_string_case_xxhash(const char* str);

// Case-folded FNV-1a over an explicit length, finished with the xxHash32
// avalanche. Apex field access is case insensitive, so every field lookup
// needs a lowercased hash; this computes one without allocating a lowercased
// copy of the name first (which is what the helpers above do, per call).
uint32_t nadr_hash_fold(const char* s, size_t len);

#endif // NADIR_HASH_H
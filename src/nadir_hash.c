// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime thingy

#include "nadir_hash.h"
#include <ctype.h>

#define ROTL32(x, r) (((x) << (r)) | ((x) >> (32 - (r))))
#define ROTL64(x, r) (((x) << (r)) | ((x) >> (64 - (r))))

// -----------------------------------------------------------------------------
// MurmurHash3 (32-bit and 128-bit)
// -----------------------------------------------------------------------------

static inline uint32_t fmix32(uint32_t h) {
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

uint32_t murmur3_32(const void* key, size_t len, uint32_t seed) {
    const uint8_t* data = (const uint8_t*)key;
    const int nblocks = (int)(len / 4);
    uint32_t h1 = seed;

    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;

    // Body: 4-byte chunks
    const uint32_t* blocks = (const uint32_t*)(data + nblocks * 4);
    for (int i = -nblocks; i; i++) {
        uint32_t k1;
        memcpy(&k1, &blocks[i], sizeof(uint32_t));

        k1 *= c1;
        k1 = ROTL32(k1, 15);
        k1 *= c2;

        h1 ^= k1;
        h1 = ROTL32(h1, 13);
        h1 = h1 * 5 + 0xe6546b64;
    }

    // Tail
    const uint8_t* tail = (const uint8_t*)(data + nblocks * 4);
    uint32_t k1 = 0;

    switch (len & 3) {
        case 3: k1 ^= (uint32_t)tail[2] << 16; // fallthrough
        case 2: k1 ^= (uint32_t)tail[1] << 8;  // fallthrough
        case 1: k1 ^= (uint32_t)tail[0];
                k1 *= c1; k1 = ROTL32(k1, 15); k1 *= c2; h1 ^= k1;
    }

    // Finalization
    h1 ^= (uint32_t)len;
    h1 = fmix32(h1);
    return h1;
}

static inline uint64_t fmix64(uint64_t k) {
    k ^= k >> 33;
    k *= 0xff51afd7ed558ccdULL;
    k ^= k >> 33;
    k *= 0xc4ceb9fe1a85ec53ULL;
    k ^= k >> 33;
    return k;
}

void murmur3_128(const void* key, size_t len, uint32_t seed, void* out) {
    const uint8_t* data = (const uint8_t*)key;
    const int nblocks = (int)(len / 16);

    uint64_t h1 = seed;
    uint64_t h2 = seed;

    const uint64_t c1 = 0x87c37b91114253d5ULL;
    const uint64_t c2 = 0x4cf5ad432745937fULL;

    const uint64_t* blocks = (const uint64_t*)(data);
    for (int i = 0; i < nblocks; i++) {
        uint64_t k1, k2;
        memcpy(&k1, &blocks[i * 2 + 0], sizeof(uint64_t));
        memcpy(&k2, &blocks[i * 2 + 1], sizeof(uint64_t));

        k1 *= c1; k1 = ROTL64(k1, 31); k1 *= c2; h1 ^= k1;
        h1 = ROTL64(h1, 27); h1 += h2; h1 = h1 * 5 + 0x52dce729;

        k2 *= c2; k2 = ROTL64(k2, 33); k2 *= c1; h2 ^= k2;
        h2 = ROTL64(h2, 31); h2 += h1; h2 = h2 * 5 + 0x38495ab5;
    }

    const uint8_t* tail = (const uint8_t*)(data + nblocks * 16);
    uint64_t k1 = 0;
    uint64_t k2 = 0;

    switch (len & 15) {
        case 15: k2 ^= ((uint64_t)tail[14]) << 48; // fallthrough
        case 14: k2 ^= ((uint64_t)tail[13]) << 40; // fallthrough
        case 13: k2 ^= ((uint64_t)tail[12]) << 32; // fallthrough
        case 12: k2 ^= ((uint64_t)tail[11]) << 24; // fallthrough
        case 11: k2 ^= ((uint64_t)tail[10]) << 16; // fallthrough
        case 10: k2 ^= ((uint64_t)tail[ 9]) << 8;  // fallthrough
        case  9: k2 ^= ((uint64_t)tail[ 8]) << 0;
                 k2 *= c2; k2 = ROTL64(k2, 33); k2 *= c1; h2 ^= k2; // fallthrough
        case  8: k1 ^= ((uint64_t)tail[ 7]) << 56; // fallthrough
        case  7: k1 ^= ((uint64_t)tail[ 6]) << 48; // fallthrough
        case  6: k1 ^= ((uint64_t)tail[ 5]) << 40; // fallthrough
        case  5: k1 ^= ((uint64_t)tail[ 4]) << 32; // fallthrough
        case  4: k1 ^= ((uint64_t)tail[ 3]) << 24; // fallthrough
        case  3: k1 ^= ((uint64_t)tail[ 2]) << 16; // fallthrough
        case  2: k1 ^= ((uint64_t)tail[ 1]) << 8;  // fallthrough
        case  1: k1 ^= ((uint64_t)tail[ 0]) << 0;
                 k1 *= c1; k1 = ROTL64(k1, 31); k1 *= c2; h1 ^= k1;
    }

    h1 ^= len; h2 ^= len;
    h1 += h2; h2 += h1;
    h1 = fmix64(h1); h2 = fmix64(h2);
    h1 += h2; h2 += h1;

    uint64_t* out64 = (uint64_t*)out;
    out64[0] = h1;
    out64[1] = h2;
}

// -----------------------------------------------------------------------------
// xxHash (xxHash32 and xxHash64)
// -----------------------------------------------------------------------------

static const uint32_t PRIME32_1 = 2654435761U;
static const uint32_t PRIME32_2 = 2246822519U;
static const uint32_t PRIME32_3 = 3266489917U;
static const uint32_t PRIME32_4 =  668265263U;
static const uint32_t PRIME32_5 =  374761393U;

uint32_t xxhash32(const void* input, size_t len, uint32_t seed) {
    const uint8_t* p = (const uint8_t*)input;
    const uint8_t* const bEnd = p + len;
    uint32_t h32;

    if (len >= 16) {
        const uint8_t* const limit = bEnd - 16;
        uint32_t v1 = seed + PRIME32_1 + PRIME32_2;
        uint32_t v2 = seed + PRIME32_2;
        uint32_t v3 = seed + 0;
        uint32_t v4 = seed - PRIME32_1;

        do {
            uint32_t k1, k2, k3, k4;
            memcpy(&k1, p + 0, 4);
            memcpy(&k2, p + 4, 4);
            memcpy(&k3, p + 8, 4);
            memcpy(&k4, p + 12, 4);

            v1 = ROTL32(v1 + k1 * PRIME32_2, 13) * PRIME32_1;
            v2 = ROTL32(v2 + k2 * PRIME32_2, 13) * PRIME32_1;
            v3 = ROTL32(v3 + k3 * PRIME32_2, 13) * PRIME32_1;
            v4 = ROTL32(v4 + k4 * PRIME32_2, 13) * PRIME32_1;
            p += 16;
        } while (p <= limit);

        h32 = ROTL32(v1, 1) + ROTL32(v2, 7) + ROTL32(v3, 12) + ROTL32(v4, 18);
    } else {
        h32 = seed + PRIME32_5;
    }

    h32 += (uint32_t)len;

    while (p + 4 <= bEnd) {
        uint32_t k1;
        memcpy(&k1, p, 4);
        h32 += k1 * PRIME32_3;
        h32 = ROTL32(h32, 17) * PRIME32_4;
        p += 4;
    }

    while (p < bEnd) {
        h32 += (*p) * PRIME32_5;
        h32 = ROTL32(h32, 11) * PRIME32_1;
        p++;
    }

    h32 ^= h32 >> 15;
    h32 *= PRIME32_2;
    h32 ^= h32 >> 13;
    h32 *= PRIME32_3;
    h32 ^= h32 >> 16;

    return h32;
}

static const uint64_t PRIME64_1 = 11400714785074694791ULL;
static const uint64_t PRIME64_2 = 14029467366897019727ULL;
static const uint64_t PRIME64_3 =  1609587929392839161ULL;
static const uint64_t PRIME64_4 =  9650029242287828579ULL;
static const uint64_t PRIME64_5 =  2870177450012600261ULL;

uint64_t xxhash64(const void* input, size_t len, uint64_t seed) {
    const uint8_t* p = (const uint8_t*)input;
    const uint8_t* const bEnd = p + len;
    uint64_t h64;

    if (len >= 32) {
        const uint8_t* const limit = bEnd - 32;
        uint64_t v1 = seed + PRIME64_1 + PRIME64_2;
        uint64_t v2 = seed + PRIME64_2;
        uint64_t v3 = seed + 0;
        uint64_t v4 = seed - PRIME64_1;

        do {
            uint64_t k1, k2, k3, k4;
            memcpy(&k1, p + 0, 8);
            memcpy(&k2, p + 8, 8);
            memcpy(&k3, p + 16, 8);
            memcpy(&k4, p + 24, 8);

            v1 = ROTL64(v1 + k1 * PRIME64_2, 31) * PRIME64_1;
            v2 = ROTL64(v2 + k2 * PRIME64_2, 31) * PRIME64_1;
            v3 = ROTL64(v3 + k3 * PRIME64_2, 31) * PRIME64_1;
            v4 = ROTL64(v4 + k4 * PRIME64_2, 31) * PRIME64_1;
            p += 32;
        } while (p <= limit);

        h64 = ROTL64(v1, 1) + ROTL64(v2, 7) + ROTL64(v3, 12) + ROTL64(v4, 18);

        v1 *= PRIME64_2; v1 = ROTL64(v1, 31); v1 *= PRIME64_1; h64 ^= v1; h64 = h64 * PRIME64_1 + PRIME64_4;
        v2 *= PRIME64_2; v2 = ROTL64(v2, 31); v2 *= PRIME64_1; h64 ^= v2; h64 = h64 * PRIME64_1 + PRIME64_4;
        v3 *= PRIME64_2; v3 = ROTL64(v3, 31); v3 *= PRIME64_1; h64 ^= v3; h64 = h64 * PRIME64_1 + PRIME64_4;
        v4 *= PRIME64_2; v4 = ROTL64(v4, 31); v4 *= PRIME64_1; h64 ^= v4; h64 = h64 * PRIME64_1 + PRIME64_4;
    } else {
        h64 = seed + PRIME64_5;
    }

    h64 += (uint64_t)len;

    while (p + 8 <= bEnd) {
        uint64_t k1;
        memcpy(&k1, p, 8);
        k1 *= PRIME64_2; k1 = ROTL64(k1, 31); k1 *= PRIME64_1; h64 ^= k1;
        h64 = ROTL64(h64, 27) * PRIME64_1 + PRIME64_4;
        p += 8;
    }

    if (p + 4 <= bEnd) {
        uint32_t k1;
        memcpy(&k1, p, 4);
        h64 ^= (uint64_t)k1 * PRIME64_1;
        h64 = ROTL64(h64, 23) * PRIME64_2 + PRIME64_3;
        p += 4;
    }

    while (p < bEnd) {
        h64 ^= ((uint64_t)(*p)) * PRIME64_5;
        h64 = ROTL64(h64, 11) * PRIME64_1;
        p++;
    }

    h64 ^= h64 >> 33;
    h64 *= PRIME64_2;
    h64 ^= h64 >> 29;
    h64 *= PRIME64_3;
    h64 ^= h64 >> 32;

    return h64;
}

// -----------------------------------------------------------------------------
// Case-Insensitive String Hashes for Apex
// -----------------------------------------------------------------------------

uint32_t hash_string_case_murmur3(const char* str) {
    if (!str) return 0;
    size_t len = strlen(str);
    char buf[256];
    char* target = buf;
    if (len >= sizeof(buf)) {
        target = (char*)malloc(len + 1);
    }
    for (size_t i = 0; i < len; i++) {
        target[i] = (char)tolower((unsigned char)str[i]);
    }
    target[len] = '\0';

    uint32_t hash = murmur3_32(target, len, 0x9747b28c);
    if (target != buf) free(target);
    return hash;
}

uint32_t hash_string_case_xxhash(const char* str) {
    if (!str) return 0;
    size_t len = strlen(str);
    char buf[256];
    char* target = buf;
    if (len >= sizeof(buf)) {
        target = (char*)malloc(len + 1);
    }
    for (size_t i = 0; i < len; i++) {
        target[i] = (char)tolower((unsigned char)str[i]);
    }
    target[len] = '\0';

    uint32_t hash = xxhash32(target, len, 0);
    if (target != buf) free(target);
    return hash;
}

// Case-folded FNV-1a over [s, s+len), finished with the xxHash32 avalanche so
// the low bits are well distributed for power-of-two table masking.
// No allocation: folding happens on the fly, one byte at a time.
uint32_t nadr_hash_fold(const char* s, size_t len) {
    uint32_t h = 2166136261u;
    if (!s) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        // ASCII case fold only: Apex identifiers/metadata names are ASCII and
        // this avoids the locale-dependent tolower table lookup per byte.
        if (c >= 'A' && c <= 'Z') c = (unsigned char)(c + 32);
        h ^= c;
        h *= 16777619u;
    }
    // xxHash32 avalanche
    h ^= h >> 15;
    h *= PRIME32_2;
    h ^= h >> 13;
    h *= PRIME32_3;
    h ^= h >> 16;
    return h;
}
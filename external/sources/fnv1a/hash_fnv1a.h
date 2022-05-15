#pragma once
#include <stdint.h>

//fnv1a 32 and 64 bit hash functions
// key is the data to hash, len is the size of the data (or how much of it to hash against)
// code license: public domain or equivalent
// post: https://notes.underscorediscovery.com/constexpr-fnv1a/

inline uint32_t hash_32_fnv1a(const void* key, const unsigned int len) {

	const unsigned char* data = static_cast<const unsigned char*>(key);
    uint32_t hash = 0x811c9dc5;
    uint32_t prime = 0x1000193;

	for(unsigned int i = 0; i < len; ++i) {
		const unsigned char value = data[i];
        hash = hash ^ value;
        hash *= prime;
    }

    return hash;

} //hash_32_fnv1a

inline uint64_t hash_64_fnv1a(const void* key, const unsigned int len) {
    
	const unsigned char* data = static_cast<const unsigned char*>(key);
    uint64_t hash = 0xcbf29ce484222325;
    uint64_t prime = 0x100000001b3;

	for(unsigned int i = 0; i < len; ++i) {
        uint8_t value = data[i];
        hash = hash ^ value;
        hash *= prime;
    }

    return hash;

} //hash_64_fnv1a

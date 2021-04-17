#pragma once

#include <cinttypes>

// don't use these in your code
using raw_s8 = int8_t;
using raw_u8 = uint8_t;
using raw_s16 = int16_t;
using raw_u16 = uint16_t;
using raw_s32 = int32_t;
using raw_u32 = uint32_t;
using raw_s64 = int64_t;
using raw_u64 = uint64_t;
using raw_f32 = float;
using raw_f64 = double;

// can be aliased to some other types in debug mode for overflows checking and other debug stuff
using s8 = raw_s8;
using u8 = raw_u8;
using s16 = raw_s16;
using u16 = raw_u16;
using s32 = raw_s32;
using u32 = raw_u32;
using s64 = raw_s64;
using u64 = raw_u64;
using f32 = raw_f32;
using f64 = raw_f64;

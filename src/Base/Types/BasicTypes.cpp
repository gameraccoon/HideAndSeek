#include "Base/precomp.h"

#include "Base/Types/BasicTypes.h"

#include <type_traits>

static_assert(sizeof(s8) == 1, "Wrong size of s8");
static_assert(std::is_signed<raw_s8>(), "Should be signed");
static_assert(std::is_integral<raw_s8>(), "Should be integral");

static_assert(sizeof(u8) == 1, "Wrong size of type");
static_assert(std::is_unsigned<raw_u8>(), "Should be unsigned");
static_assert(std::is_integral<raw_u8>(), "Should be integral");

static_assert(sizeof(s16) == 2, "Wrong size of type");
static_assert(std::is_signed<raw_s16>(), "Should be signed");
static_assert(std::is_integral<raw_s16>(), "Should be integral");

static_assert(sizeof(u16) == 2, "Wrong size of type");
static_assert(std::is_unsigned<raw_u16>(), "Should be unsigned");
static_assert(std::is_integral<raw_u16>(), "Should be integral");

static_assert(sizeof(s32) == 4, "Wrong size of type");
static_assert(std::is_signed<raw_s32>(), "Should be signed");
static_assert(std::is_integral<raw_s32>(), "Should be integral");

static_assert(sizeof(u32) == 4, "Wrong size of type");
static_assert(std::is_unsigned<raw_u32>(), "Should be unsigned");
static_assert(std::is_integral<raw_u32>(), "Should be integral");

static_assert(sizeof(s64) == 8, "Wrong size of type");
static_assert(std::is_signed<raw_s64>(), "Should be signed");
static_assert(std::is_integral<raw_s64>(), "Should be integral");

static_assert(sizeof(u64) == 8, "Wrong size of type");
static_assert(std::is_unsigned<raw_u64>(), "Should be unsigned");
static_assert(std::is_integral<raw_u64>(), "Should be integral");

static_assert(sizeof(f32) == 4, "Wrong size of type");
static_assert(std::is_floating_point<raw_f32>(), "Should be floating point");

static_assert(sizeof(f64) == 8, "Wrong size of type");
static_assert(std::is_floating_point<raw_f64>(), "Should be floating point");




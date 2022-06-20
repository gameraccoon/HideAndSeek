#include "Base/precomp.h"

#include <gtest/gtest.h>

#include "Base/Types/BasicTypes.h"
#include "Base/Types/Serialization.h"

TEST(BasicTypesSerialization, WriteNumber_Size)
{
	std::vector<std::byte> stream;
	stream.reserve(8);

	Serialization::WriteNumberNarrowCast<u8>(stream, 5u);
	EXPECT_EQ(static_cast<size_t>(1), stream.size());
	stream.clear();

	Serialization::WriteNumberNarrowCast<u16>(stream, 5u);
	EXPECT_EQ(static_cast<size_t>(2), stream.size());
	stream.clear();

	Serialization::WriteNumberNarrowCast<u32>(stream, 5u);
	EXPECT_EQ(static_cast<size_t>(4), stream.size());
	stream.clear();

	Serialization::WriteNumberWideCast<u64>(stream, 5u);
	EXPECT_EQ(static_cast<size_t>(8), stream.size());
	stream.clear();

	Serialization::WriteNumberNarrowCast<f32>(stream, 5.0f);
	EXPECT_EQ(static_cast<size_t>(4), stream.size());
	stream.clear();

	Serialization::WriteNumber<f64>(stream, 5.0);
	EXPECT_EQ(static_cast<size_t>(8), stream.size());
	stream.clear();
}

TEST(BasicTypesSerialization, WriteNumber_ReadNumber)
{
	std::vector<std::byte> stream;

	const s8 testSignedInt8 = -4;
	const s16 testSignedInt16 = -3;
	const s32 testSignedInt32 = -2;
	const s64 testSignedInt64 = -1;
	const u8 testUnsignedInt8 = 1u;
	const u16 testUnsignedInt16 = 2u;
	const u32 testUnsignedInt32 = 3u;
	const u64 testUnsignedInt64 = 4u;
	const f32 testFloat32 = -5.0f;
	const f64 testFloat64 = 6.0;

	Serialization::WriteNumber<s8>(stream, testSignedInt8);
	Serialization::WriteNumber<s16>(stream, testSignedInt16);
	Serialization::WriteNumber<s32>(stream, testSignedInt32);
	Serialization::WriteNumber<s64>(stream, testSignedInt64);
	Serialization::WriteNumber<u8>(stream, testUnsignedInt8);
	Serialization::WriteNumber<u16>(stream, testUnsignedInt16);
	Serialization::WriteNumber<u32>(stream, testUnsignedInt32);
	Serialization::WriteNumber<u64>(stream, testUnsignedInt64);
	Serialization::WriteNumber<f32>(stream, testFloat32);
	Serialization::WriteNumber<f64>(stream, testFloat64);
	EXPECT_EQ(size_t(1+2+4+8+1+2+4+8+4+8), stream.size());

	size_t cursorPos = 0;
	EXPECT_EQ(static_cast<s8>(-4), Serialization::ReadNumber<s8>(stream, cursorPos));
	EXPECT_EQ(static_cast<s16>(-3), Serialization::ReadNumber<s16>(stream, cursorPos));
	EXPECT_EQ(static_cast<s32>(-2), Serialization::ReadNumber<s32>(stream, cursorPos));
	EXPECT_EQ(static_cast<s64>(-1), Serialization::ReadNumber<s64>(stream, cursorPos));
	EXPECT_EQ(static_cast<u8>(1), Serialization::ReadNumber<u8>(stream, cursorPos));
	EXPECT_EQ(static_cast<u16>(2), Serialization::ReadNumber<u16>(stream, cursorPos));
	EXPECT_EQ(static_cast<u32>(3), Serialization::ReadNumber<u32>(stream, cursorPos));
	EXPECT_EQ(static_cast<u64>(4), Serialization::ReadNumber<u64>(stream, cursorPos));
	EXPECT_EQ(static_cast<f32>(-5.0f), Serialization::ReadNumber<f32>(stream, cursorPos));
	EXPECT_EQ(static_cast<f64>(6.0), Serialization::ReadNumber<f64>(stream, cursorPos));
}

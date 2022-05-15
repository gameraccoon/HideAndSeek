#include "Base/precomp.h"

#include <gtest/gtest.h>

#include "Base/Types/BasicTypes.h"
#include "Base/Types/Serialization.h"

TEST(BasicTypesSerialization, WriteNumber_Size)
{
	std::vector<std::byte> stream;
	stream.reserve(8);

	Serialization::WriteNumber<u8>(stream, 5);
	EXPECT_EQ(static_cast<size_t>(1), stream.size());
	stream.clear();

	Serialization::WriteNumber<u16>(stream, 5);
	EXPECT_EQ(static_cast<size_t>(2), stream.size());
	stream.clear();

	Serialization::WriteNumber<u32>(stream, 5);
	EXPECT_EQ(static_cast<size_t>(4), stream.size());
	stream.clear();

	Serialization::WriteNumber<u64>(stream, 5);
	EXPECT_EQ(static_cast<size_t>(8), stream.size());
	stream.clear();

	Serialization::WriteNumber<f32>(stream, 5);
	EXPECT_EQ(static_cast<size_t>(4), stream.size());
	stream.clear();

	Serialization::WriteNumber<f64>(stream, 5);
	EXPECT_EQ(static_cast<size_t>(8), stream.size());
	stream.clear();
}

TEST(BasicTypesSerialization, WriteNumber_ReadNumber)
{
	std::vector<std::byte> stream;

	Serialization::WriteNumber<s8>(stream, -4);
	Serialization::WriteNumber<s16>(stream, -3);
	Serialization::WriteNumber<s32>(stream, -2);
	Serialization::WriteNumber<s64>(stream, -1);
	Serialization::WriteNumber<u8>(stream, 1);
	Serialization::WriteNumber<u16>(stream, 2);
	Serialization::WriteNumber<u32>(stream, 3);
	Serialization::WriteNumber<u64>(stream, 4);
	Serialization::WriteNumber<f32>(stream, -5.0f);
	Serialization::WriteNumber<f64>(stream, 6.0);
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

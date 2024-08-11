#include "EngineCommon/precomp.h"

#include <gtest/gtest.h>

#include "EngineCommon/Types/Serialization.h"

TEST(BasicTypesSerialization, WriteNumber_Position)
{
	std::vector<std::byte> stream;

	stream.assign(3, std::byte(0));
	size_t streamPos = 1;
	Serialization::WriteNumber<u8>(stream, static_cast<u8>(0xFFu), streamPos);
	EXPECT_EQ(std::vector<std::byte>({
		std::byte(0),
		std::byte(0xFF),
		std::byte(0)
	}), stream);

	stream.assign(4, std::byte(0));
	streamPos = 1;
	Serialization::WriteNumber<u16>(stream, static_cast<u16>(0xFFFFu), streamPos);
	EXPECT_EQ(std::vector<std::byte>({
		std::byte(0),
		std::byte(0xFF),
		std::byte(0xFF),
		std::byte(0)
	}), stream);

	stream.assign(6, std::byte(0));
	streamPos = 1;
	Serialization::WriteNumber<u32>(stream, static_cast<u32>(0xFFFFFFFFu), streamPos);
	EXPECT_EQ(std::vector<std::byte>({
		std::byte(0),
		std::byte(0xFF),
		std::byte(0xFF),
		std::byte(0xFF),
		std::byte(0xFF),
		std::byte(0)
	}), stream);

	stream.assign(10, std::byte(0));
	streamPos = 1;
	Serialization::WriteNumber<u64>(stream, static_cast<u64>(0xFFFFFFFFFFFFFFFFull), streamPos);
	EXPECT_EQ(std::vector<std::byte>({
		std::byte(0),
		std::byte(0xFF),
		std::byte(0xFF),
		std::byte(0xFF),
		std::byte(0xFF),
		std::byte(0xFF),
		std::byte(0xFF),
		std::byte(0xFF),
		std::byte(0xFF),
		std::byte(0)
	}), stream);

	stream.assign(6, std::byte(0));
	streamPos = 1;
	Serialization::WriteNumber<f32>(stream, 2.36942782762e-38f, streamPos);
	EXPECT_EQ(std::vector<std::byte>({
		std::byte(0),
		std::byte(0x01),
		std::byte(0x01),
		std::byte(0x01),
		std::byte(0x01),
		std::byte(0)
	}), stream);

	stream.assign(10, std::byte(0));
	streamPos = 1;
	Serialization::WriteNumber<f64>(stream, 7.748604185489348e-304, streamPos);
	EXPECT_EQ(std::vector<std::byte>({
		std::byte(0),
		std::byte(0x01),
		std::byte(0x01),
		std::byte(0x01),
		std::byte(0x01),
		std::byte(0x01),
		std::byte(0x01),
		std::byte(0x01),
		std::byte(0x01),
		std::byte(0)
	}), stream);
}

TEST(BasicTypesSerialization, AppendNumber_Size)
{
	std::vector<std::byte> stream;
	stream.reserve(8);

	Serialization::AppendNumberNarrowCast<u8>(stream, 5u);
	EXPECT_EQ(static_cast<size_t>(1), stream.size());
	stream.clear();

	Serialization::AppendNumberNarrowCast<u16>(stream, 5u);
	EXPECT_EQ(static_cast<size_t>(2), stream.size());
	stream.clear();

	Serialization::AppendNumberNarrowCast<u32>(stream, 5u);
	EXPECT_EQ(static_cast<size_t>(4), stream.size());
	stream.clear();

	Serialization::AppendNumberWideCast<u64>(stream, 5u);
	EXPECT_EQ(static_cast<size_t>(8), stream.size());
	stream.clear();

	Serialization::AppendNumberNarrowCast<f32>(stream, 5.0f);
	EXPECT_EQ(static_cast<size_t>(4), stream.size());
	stream.clear();

	Serialization::AppendNumber<f64>(stream, 5.0);
	EXPECT_EQ(static_cast<size_t>(8), stream.size());
	stream.clear();
}

TEST(BasicTypesSerialization, WriteNumber_ReadNumber)
{
	std::vector<std::byte> stream;
	stream.resize(1 + 2 + 4 + 8 + 1 + 2 + 4 + 8 + 4 + 8);

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

	size_t streamPos = 0;
	Serialization::WriteNumber<s8>(stream, testSignedInt8, streamPos);
	Serialization::WriteNumber<s16>(stream, testSignedInt16, streamPos);
	Serialization::WriteNumber<s32>(stream, testSignedInt32, streamPos);
	Serialization::WriteNumber<s64>(stream, testSignedInt64, streamPos);
	Serialization::WriteNumber<u8>(stream, testUnsignedInt8, streamPos);
	Serialization::WriteNumber<u16>(stream, testUnsignedInt16, streamPos);
	Serialization::WriteNumber<u32>(stream, testUnsignedInt32, streamPos);
	Serialization::WriteNumber<u64>(stream, testUnsignedInt64, streamPos);
	Serialization::WriteNumber<f32>(stream, testFloat32, streamPos);
	Serialization::WriteNumber<f64>(stream, testFloat64, streamPos);

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

TEST(BasicTypesSerialization, AppendNumber_ReadNumber)
{
	std::vector<std::byte> stream;

	constexpr s8 testSignedInt8 = -4;
	constexpr s16 testSignedInt16 = -3;
	constexpr s32 testSignedInt32 = -2;
	constexpr s64 testSignedInt64 = -1;
	constexpr u8 testUnsignedInt8 = 1u;
	constexpr u16 testUnsignedInt16 = 2u;
	constexpr u32 testUnsignedInt32 = 3u;
	constexpr u64 testUnsignedInt64 = 4u;
	constexpr f32 testFloat32 = -5.0f;
	constexpr f64 testFloat64 = 6.0;

	Serialization::AppendNumber<s8>(stream, testSignedInt8);
	Serialization::AppendNumber<s16>(stream, testSignedInt16);
	Serialization::AppendNumber<s32>(stream, testSignedInt32);
	Serialization::AppendNumber<s64>(stream, testSignedInt64);
	Serialization::AppendNumber<u8>(stream, testUnsignedInt8);
	Serialization::AppendNumber<u16>(stream, testUnsignedInt16);
	Serialization::AppendNumber<u32>(stream, testUnsignedInt32);
	Serialization::AppendNumber<u64>(stream, testUnsignedInt64);
	Serialization::AppendNumber<f32>(stream, testFloat32);
	Serialization::AppendNumber<f64>(stream, testFloat64);
	EXPECT_EQ(size_t(1 + 2 + 4 + 8 + 1 + 2 + 4 + 8 + 4 + 8), stream.size());

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

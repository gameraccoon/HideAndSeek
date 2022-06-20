#pragma once

#include <array>
#include <bit>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace Serialization
{
	using ByteStream = std::vector<std::byte>;

	template<typename Num, typename NumArg>
	void WriteNumber(std::vector<std::byte>& inOutByteStream, NumArg number)
	{
		static_assert(std::is_same_v<typename std::decay<NumArg>::type, Num>, "We should provide argument of the same type that we want to write. If you want to make conversion, you can use WriteNumberNarrowCast or WriteNumberWideCast");
		static_assert(std::is_arithmetic_v<Num>, "Type should be ariphmetic to be serialized with WriteNumber");
		static_assert(sizeof(std::array<std::byte, sizeof(Num)>) == sizeof(Num), "Unexpected std::array layout");
		static_assert(std::is_standard_layout_v<std::array<std::byte, sizeof(Num)>>, "Unexpected std::array layout");
		static_assert(std::is_trivially_constructible_v<std::array<std::byte, sizeof(Num)>>, "Unexpected std::array implementation");
		static_assert(std::is_trivially_copyable_v<std::array<std::byte, sizeof(Num)>>, "Unexpected std::array implementation");

		const auto* byteRepresentation = std::bit_cast<std::array<std::byte, sizeof(Num)>*>(&number);

		if constexpr (std::endian::native == std::endian::little)
		{
			inOutByteStream.insert(
				inOutByteStream.end(),
				std::begin(*byteRepresentation),
				std::end(*byteRepresentation)
			);
		}
		else if constexpr (std::endian::native == std::endian::big)
		{
			inOutByteStream.insert(
				inOutByteStream.end(),
				std::rbegin(*byteRepresentation),
				std::rend(*byteRepresentation)
			);
		}
		else
		{
			throw std::logic_error("Mixed entian is not supported");
		}
	}

	template<typename Num, typename NumArg>
	void WriteNumberNarrowCast(std::vector<std::byte>& inOutByteStream, NumArg number)
	{
		static_assert(std::is_convertible_v<NumArg, Num>, "Argument type should be convertible to the data type");
		static_assert(sizeof(NumArg) >= sizeof(Num), "WriteNumberNarrowCast called with a value of smaller type, that may be a sign of a logical error or inefficient use of the stream space");
		static_assert(std::is_signed_v<NumArg> == std::is_signed_v<Num>, "The provided type has different signess");

		WriteNumber<Num>(inOutByteStream, static_cast<Num>(number));
	}

	template<typename Num, typename NumArg>
	void WriteNumberWideCast(std::vector<std::byte>& inOutByteStream, NumArg number)
	{
		static_assert(std::is_convertible_v<NumArg, Num>, "Argument type should be convertible to the data type");
		static_assert(sizeof(NumArg) <= sizeof(Num), "WriteNumberWideCast called with a value of bigger type, that may be a sign of a logical error or potential data loss");
		static_assert(std::is_signed_v<NumArg> == std::is_signed_v<Num>, "The provided type has different signess");

		WriteNumber<Num>(inOutByteStream, static_cast<Num>(number));
	}

	template<typename Num>
	Num ReadNumber(const ByteStream& inOutByteStream, size_t& cursorPos)
	{
		static_assert(std::is_arithmetic_v<Num>, "Type should be ariphmetic to be deserialized with ReadNumber");
		static_assert(sizeof(std::array<std::byte, sizeof(Num)>) == sizeof(Num), "Unexpected std::array layout");
		static_assert(std::is_standard_layout_v<std::array<std::byte, sizeof(Num)>>, "Unexpected std::array layout");
		static_assert(std::is_trivially_constructible_v<std::array<std::byte, sizeof(Num)>>, "Unexpected std::array implementation");
		static_assert(std::is_trivially_copyable_v<std::array<std::byte, sizeof(Num)>>, "Unexpected std::array implementation");

		Num number;
		auto* byteRepresentation = std::bit_cast<std::array<std::byte, sizeof(Num)>*>(&number);

		const size_t lastCursorPos = cursorPos;

		if constexpr (std::endian::native == std::endian::little)
		{
			std::copy(
				inOutByteStream.begin() + lastCursorPos,
				inOutByteStream.begin() + (lastCursorPos + sizeof(Num)),
				byteRepresentation->begin()
			);
		}
		else if constexpr (std::endian::native == std::endian::big)
		{
			std::copy(
				inOutByteStream.rbegin() + (inOutByteStream.size() - lastCursorPos - sizeof(Num)),
				inOutByteStream.rbegin() + (inOutByteStream.size() - lastCursorPos),
				byteRepresentation->begin()
			);
		}
		else
		{
			throw std::logic_error("Mixed entian is not supported");
		}

		cursorPos += sizeof(Num);
		return number;
	}
}

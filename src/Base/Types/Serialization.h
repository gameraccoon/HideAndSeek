#pragma once

#include <vector>
#include <cstddef>
#include <optional>
#include <cmath>

#include "Base/Debug/Assert.h"
#include "Base/Types/BasicTypes.h"
#include "Base/Types/UnsafeSerialization.h"

namespace Serialization
{
	template<typename Num>
	bool IsGoodSerializableValue(Num number)
	{
		if constexpr (std::is_floating_point_v<Num>)
		{
			if (!std::isfinite(number))
			{
				ReportError("The read value was NaN or infinite");
				return false;
			}
		}

		return true;
	}

	template<typename Num, typename ByteStream>
	bool IsValueInBounds(const ByteStream& inOutByteStream, size_t& cursorPos)
	{
		if (cursorPos + sizeof(Num) > inOutByteStream.size())
		{
			ReportError("The buffer is too short to fit the provided type (%u + %u <= %u)", cursorPos, sizeof(Num), inOutByteStream.size());
			return false;
		}

		return true;
	}

	template<typename Num, typename NumArg>
	bool AppendNumber(std::vector<std::byte>& inOutByteStream, NumArg number)
	{
		if (!IsGoodSerializableValue<Num>(number))
		{
			return false;
		}

		UnsafeSerialization::AppendNumber<Num, NumArg>(inOutByteStream, number);
		return true;
	}

	template<typename Num, typename NumArg, typename ByteStream>
	bool WriteNumber(ByteStream& inOutByteStream, NumArg number, size_t& cursorPos)
	{
		if (!IsValueInBounds<Num, ByteStream>(inOutByteStream, cursorPos))
		{
			return false;
		}

		if (!IsGoodSerializableValue<Num>(number))
		{
			return false;
		}

		UnsafeSerialization::WriteNumber<Num, NumArg, ByteStream>(inOutByteStream, number, cursorPos);
		return true;
	}

	template<typename Num, typename NumArg>
	bool AppendNumberNarrowCast(std::vector<std::byte>& inOutByteStream, NumArg number)
	{
		if (!IsGoodSerializableValue(number))
		{
			return false;
		}

		UnsafeSerialization::AppendNumberNarrowCast<Num, NumArg>(inOutByteStream, number);
		return true;
	}

	template<typename Num, typename NumArg>
	bool AppendNumberWideCast(std::vector<std::byte>& inOutByteStream, NumArg number)
	{
		if (!IsGoodSerializableValue(number))
		{
			return false;
		}

		UnsafeSerialization::AppendNumberWideCast<Num, NumArg>(inOutByteStream, number);
		return true;
	}

	template<typename Num, typename NumArg, typename ByteStream>
	bool WriteNumberNarrowCast(std::vector<std::byte>& inOutByteStream, NumArg number, size_t& cursorPos)
	{
		if (!IsValueInBounds<Num, ByteStream>(inOutByteStream, cursorPos))
		{
			return false;
		}

		if (!IsGoodSerializableValue(number))
		{
			return false;
		}

		UnsafeSerialization::WriteNumberNarrowCast<Num, NumArg, ByteStream>(inOutByteStream, number, cursorPos);
		return true;
	}

	template<typename Num, typename NumArg, typename ByteStream>
	bool WriteNumberWideCast(std::vector<std::byte>& inOutByteStream, NumArg number, size_t& cursorPos)
	{
		if (!IsValueInBounds<Num, ByteStream>(inOutByteStream, cursorPos))
		{
			return false;
		}

		if (!IsGoodSerializableValue(number))
		{
			return false;
		}

		UnsafeSerialization::WriteNumberWideCast<Num, NumArg, ByteStream>(inOutByteStream, number, cursorPos);
		return true;
	}

	template<typename Num, typename ByteStream>
	std::optional<Num> ReadNumber(const ByteStream& inOutByteStream, size_t& cursorPos)
	{
		if (!IsValueInBounds<Num, ByteStream>(inOutByteStream, cursorPos))
		{
			return std::nullopt;
		}

		const Num number = UnsafeSerialization::ReadNumber<Num, ByteStream>(inOutByteStream, cursorPos);

		if (!IsGoodSerializableValue<Num>(number))
		{
			return std::nullopt;
		}

		return number;
	}
}

#pragma once

#include <limits>
#include <type_traits>

namespace Utils
{
	template<typename I, typename F>
	I CompressNormalizedFloatToIntCL(F unitValue, unsigned int bitsCount)
	{
		static_assert(std::is_integral<I>() && std::is_unsigned<I>(), "CompressNormalizedFloatToIntCL can be used only with unsigned integers");
		static_assert(std::is_floating_point<F>(), "CompressNormalizedFloatToIntCL should be called on floating point");
		Assert(bitsCount <= std::numeric_limits<I>::digits, "bitsCount greater that the real bits count in the corresponding type");
		Assert(bitsCount < std::numeric_limits<F>::digits, "Using the same (or greater) amount of bits than in original float value makes no sense");

		I maxInterval = (1u << bitsCount) - 1u;
		F scaled = unitValue * static_cast<F>(maxInterval);
		I rounded = static_cast<I>(scaled + 0.5f);

		if (rounded > maxInterval)
		{
			rounded = maxInterval;
		}
		return rounded;
	}

	template<typename F, typename I>
	F DecompressNormalizedFloatFromIntCL(I compressed, unsigned int bitsCount)
	{
		static_assert(std::is_integral<I>() && std::is_unsigned<I>(), "DecompressNormalizedFloatFromIntCL can be used only called on unsigned integers");
		static_assert(std::is_floating_point<F>(), "CompressNormalizedFloatToIntCL should convert value to a floating point");
		Assert(bitsCount <= std::numeric_limits<I>::digits, "bitsCount greater that the real bits count in the corresponding type");
		Assert(bitsCount < std::numeric_limits<F>::digits, "Converting to a float with the same (or lesser) amount of bits than in the compressed value makes no sense (have you passed an incorrect type or amount of bits?)");
		I maxInterval = (1u << bitsCount) - 1u;
		F intervalSize = 1.0f / static_cast<F>(maxInterval);
		return static_cast<F>(compressed) * intervalSize;
	}

	template<typename I, typename F>
	I CompressFloatToIntCL(F value, F min, F max, unsigned int bitsCount)
	{
		return CompressNormalizedFloatToIntCL<I, F>((value - min)/(max - min), bitsCount);
	}

	template<typename F, typename I>
	F DecompressFloatFromIntCL(I compressed, F min, F max, unsigned int bitsCount)
	{
		return min + DecompressNormalizedFloatFromIntCL<F, I>(compressed, bitsCount) * (max - min);
	}
}

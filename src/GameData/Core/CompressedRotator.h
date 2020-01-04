#pragma once

#include "Base/Debug/Assert.h"
#include "Base/Math/ValueCompression.h"

#include <nlohmann/json_fwd.hpp>
#include <type_traits>

#include "GameData/Core/Rotator.h"

/**
 * Compresses rotation value into a quantized integer using RL encoding
 */
template<typename T>
class CompressedRotator
{
public:
	CompressedRotator() = default;

	explicit CompressedRotator(Rotator rotator, unsigned int bitsCount = MaxBitsCount)
		: mCompressedValue(Compress(rotator, bitsCount))
	{
	}

	Rotator decompress(unsigned int bitsCount = MaxBitsCount) const
	{
		Assert(bitsCount <= MaxBitsCount, "bitsCount greater that the real bits count in the corresponding type");
		return Decompress(mCompressedValue, bitsCount);
	}

	static T Compress(Rotator rotator, unsigned int bitsCount = MaxBitsCount)
	{
		// comvert the value to the range from 0 to max
		Rotator::UnderlyingType zeroToMaxRotation = rotator.getValue() >= 0 ? rotator.getValue() : MaxRotationFromZero-rotator.getValue();
		return Utils::CompressFloatToIntCL<T, Rotator::UnderlyingType>(zeroToMaxRotation, 0, MaxRotationFromZero, bitsCount);
	}

	static Rotator Decompress(T value, unsigned int bitsCount = MaxBitsCount)
	{
		Rotator::UnderlyingType zeroToMaxRotation = Utils::DecompressFloatFromIntCL<Rotator::UnderlyingType, T>(value, 0, MaxRotationFromZero, bitsCount);
		// comvert the value back to the Rotator value range
		return Rotator((zeroToMaxRotation <= Rotator::MaxValue) ? zeroToMaxRotation : (MaxRotationFromZero - zeroToMaxRotation));
	}

private:
	T mCompressedValue;

	static constexpr unsigned int MaxBitsCount = std::numeric_limits<T>::digits;
	// max value that we can get if we transform the range to [0;1]
	static constexpr Rotator::UnderlyingType MaxRotationFromZero = Rotator::MaxValue - Rotator::MinValue;
};

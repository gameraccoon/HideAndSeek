#pragma once

#include <nlohmann/json_fwd.hpp>
#include <type_traits>

#include "Base/Debug/Assert.h"
#include "Base/Math/ValueCompression.h"

#include "GameData/Geometry/Rotator.h"

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

	[[nodiscard]] Rotator decompress(unsigned int bitsCount = MaxBitsCount) const
	{
		Assert(bitsCount <= MaxBitsCount, "bitsCount greater that the real bits count in the corresponding type");
		return Decompress(mCompressedValue, bitsCount);
	}

	static T Compress(Rotator rotator, unsigned int bitsCount = MaxBitsCount)
	{
		// convert the value to the range from 0 to max
		float zeroToMaxRotation = rotator.getValue() >= 0 ? rotator.getValue() : MaxRotationFromZero+rotator.getValue();
		return Utils::CompressFloatToIntCL<T, float>(zeroToMaxRotation, 0, MaxRotationFromZero, bitsCount);
	}

	static Rotator Decompress(T value, unsigned int bitsCount = MaxBitsCount)
	{
		float zeroToMaxRotation = Utils::DecompressFloatFromIntCL<float, T>(value, 0, MaxRotationFromZero, bitsCount);
		// convert the value back to the Rotator value range
		// rotator will by itself normalize the value
		return Rotator((zeroToMaxRotation <= Rotator::MaxValue) ? zeroToMaxRotation : (zeroToMaxRotation-MaxRotationFromZero));
	}

private:
	T mCompressedValue;

	static constexpr unsigned int MaxBitsCount = std::numeric_limits<T>::digits;
	// max value that we can get if we transform the range to [0;1]
	static constexpr float MaxRotationFromZero = Rotator::MaxValue - Rotator::MinValue;
};

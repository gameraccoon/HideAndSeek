#pragma once

#include <cstddef>

template<size_t BitCount>
struct BitsetTraits
{
	constexpr static size_t ByteCount = BitCount / 8 + (BitCount % 8 == 0 ? 0 : 1);
};

/**
 * Simple bitset that can be serialized and deserialized
 */
template<size_t BitCount>
class SimpleBitset
{
public:
	SimpleBitset() = default;
	SimpleBitset(const SimpleBitset&) = default;
	SimpleBitset(SimpleBitset&&) = default;
	SimpleBitset& operator=(const SimpleBitset&) = default;
	SimpleBitset& operator=(SimpleBitset&&) = default;
	~SimpleBitset() = default;

	static size_t getBitCount()  noexcept { return BitCount; }

	void set(const size_t index, const bool isSet) noexcept
	{
		setInner(index, isSet);
	}

	bool get(const size_t index) const noexcept
	{
		return getInner(index);
	}

	void clear() noexcept
	{
		for (size_t i = 0; i < ByteCount; ++i)
		{
			mBitset[i] = std::byte(0);
		}
	}

	void intersect(const SimpleBitset& other) noexcept
	{
		for (size_t i = 0; i < ByteCount; ++i)
		{
			mBitset[i] &= other.mBitset[i];
		}
	}

	void unite(const SimpleBitset& other) noexcept
	{
		for (size_t i = 0; i < ByteCount; ++i)
		{
			mBitset[i] |= other.mBitset[i];
		}
	}

	void invert() noexcept
	{
		for (size_t i = 0; i < ByteCount; ++i)
		{
			mBitset[i] = ~mBitset[i];
		}
	}

	static size_t getByteCount()  noexcept { return ByteCount; }
	const std::byte* getRawData() const noexcept { return mBitset; }

	void setRawData(const std::byte* data) noexcept
	{
		for (size_t i = 0; i < ByteCount; ++i)
		{
			mBitset[i] = data[i];
		}
	}

private:
	void setInner(const size_t index, const bool isSet)
	{
		if (index >= BitCount)
		{
			ReportFatalError("Invalid index %d, max is %d", index, BitCount);
			return;
		}
		const size_t byteIndex = index / 8;
		const size_t bitIndex = index % 8;
		if (isSet)
		{
			mBitset[byteIndex] |= std::byte(1 << bitIndex);
		}
		else
		{
			mBitset[byteIndex] &= ~std::byte(1 << bitIndex);
		}
	}

	bool getInner(const size_t index) const noexcept
	{
		if (index >= BitCount)
		{
			ReportFatalError("Invalid index %d, max is %d", index, BitCount);
			return false;
		}
		const size_t byteIndex = index / 8;
		const size_t bitIndex = index % 8;
		return (mBitset[byteIndex] & std::byte(1 << bitIndex)) != std::byte(0);
	}

private:
	constexpr static size_t ByteCount = BitsetTraits<BitCount>::ByteCount;
	std::byte mBitset[ByteCount] = { std::byte(0) };
};

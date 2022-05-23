#pragma once

#include <bitset>
#include <limits>

#include <neargye/magic_enum.hpp>

/**
 * Bitset that can be used with enum class
*/
template <typename T>
class Bitset
{
public:
	Bitset() = default;
	// can be implicitly created from T
	template<typename... Args>
	explicit Bitset(Args... initialValues) { set(std::forward<Args>(initialValues)...); }

	template<typename... Args>
	void set(Args... flags) { (setInner(std::forward<Args>(flags), true), ...); }
	template<typename... Args>
	void unset(Args... flags) { (setInner(std::forward<Args>(flags), false), ...); }

	bool has(T flag) { return mBitset.test(static_cast<typename std::underlying_type_t<T>>(flag)); }
	template<typename... Args>
	bool hasAll(Args... flags) { return (has(std::forward<Args>(flags)) && ...); }
	template<typename... Args>
	bool hasAnyOf(Args... flags) { return (has(std::forward<Args>(flags)) || ...); }

private:
	void setInner(T flag, bool isSet)
	{
		mBitset.set(static_cast<typename std::underlying_type_t<T>>(flag), isSet);
	}

private:
	std::bitset<magic_enum::enum_count<T>()> mBitset;
};

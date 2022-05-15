#include "Base/precomp.h"

#include "Base/Random/RandomStrings.h"

#include <ranges>
#include <algorithm>

#include "Base/Random/Random.h"

namespace StringUtils
{
	std::string getRandomAlphanumeric(size_t length)
	{
		static constexpr char alphanumericCharacters[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";

		constexpr size_t charactersCount = sizeof(alphanumericCharacters) - 1;

		std::string result;
		result.resize(length);

		std::ranges::for_each(result, [](char& c)
		{
			c = alphanumericCharacters[Random::gGlobalGenerator() % charactersCount];
		});

		return result;
	}

	std::string getRandomBase58(size_t length)
	{
		static constexpr char base52Characters[] =
			"123456789"
			"ABCDEFGHJKLMNPQRSTUVWXYZ"
			"abcdefghijkmnopqrstuvwxyz";

		constexpr size_t charactersCount = sizeof(base52Characters) - 1;

		std::string result;
		result.resize(length);

		std::ranges::for_each(result, [](char& c)
		{
			c = base52Characters[Random::gGlobalGenerator() % charactersCount];
		});

		return result;
	}

	std::string getRandomWordSafeBase32(size_t length)
	{
		static constexpr char wordSafeBase32Characters[] =
			"23456789"
			"CFGHJMPQRVWX"
			"cfghjmpqrvwx";

		constexpr size_t charactersCount = sizeof(wordSafeBase32Characters) - 1;

		std::string result;
		result.resize(length);

		std::ranges::for_each(result, [](char& c)
		{
			c = wordSafeBase32Characters[Random::gGlobalGenerator() % charactersCount];
		});

		return result;
	}
}

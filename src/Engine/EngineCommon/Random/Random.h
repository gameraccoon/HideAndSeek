#pragma once

#include <random>

namespace Random
{
	using GlobalGeneratorType = std::mt19937;
	inline GlobalGeneratorType gGlobalGenerator(1);
}

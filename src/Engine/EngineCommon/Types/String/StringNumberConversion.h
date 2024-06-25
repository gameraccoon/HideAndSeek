#pragma once

#include <optional>

namespace String
{
	std::optional<int> ParseInt(const char* str, int base = 10);

	// will result in UB if the conversion can't be performed
	int ParseIntUnsafe(const char* str, int base = 10);
}

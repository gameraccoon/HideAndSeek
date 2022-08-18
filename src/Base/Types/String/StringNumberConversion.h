#pragma once

#include <string>
#include <optional>

namespace String
{
	std::optional<int> ParseInt(const char* str, int base = 10)
	{
		char *end;
		errno = 0;
		long l = std::strtol(str, &end, base);
		if ((errno == ERANGE && l == LONG_MAX) || l > INT_MAX) {
			return std::nullopt;
		}
		if ((errno == ERANGE && l == LONG_MIN) || l < INT_MIN) {
			return std::nullopt;
		}
		if (*str == '\0' || *end != '\0') {
			return std::nullopt;
		}
		return static_cast<int>(l);
	}

	// will result in UB if the conversion can't be performed
	int ParseIntUnsafe(const char* str, int base = 10)
	{
		[[maybe_unused]] char *end;
		errno = 0;
		[[maybe_unused]] long l = std::strtol(str, &end, base);
		Assert((errno != ERANGE || l != LONG_MAX) && l <= INT_MAX, "Overflow during string to int conversion '%s'", str);
		Assert((errno != ERANGE || l != LONG_MIN) && l >= INT_MIN, "Underflow during string to int conversion '%s'", str);
		Assert(*str != '\0' && *end == '\0', "Can't convert string '%s' to number", str);
		return static_cast<int>(l);
	}
}

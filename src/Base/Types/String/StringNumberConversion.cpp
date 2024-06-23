#include "Base/precomp.h"

#include "Base/Types/String/StringNumberConversion.h"

#include <cstdlib>

namespace String
{
	std::optional<int> ParseInt(const char* str, const int base)
	{
		char *end;
		errno = 0;
		const long l = std::strtol(str, &end, base);
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

	int ParseIntUnsafe(const char* str, const int base)
	{
		char *end;
		errno = 0;
		const long l = std::strtol(str, &end, base);
		Assert((errno != ERANGE || l != LONG_MAX) && l <= INT_MAX, "Overflow during string to int conversion '%s'", str);
		Assert((errno != ERANGE || l != LONG_MIN) && l >= INT_MIN, "Underflow during string to int conversion '%s'", str);
		Assert(*str != '\0' && *end == '\0', "Can't convert string '%s' to number", str);
		return static_cast<int>(l);
	}

} // namespace String

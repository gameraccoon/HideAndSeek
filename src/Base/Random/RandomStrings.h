#pragma once

#include <string>

namespace StringUtils
{
	std::string getRandomAlphanumeric(size_t length);
	// like alphanumeric but with less ambiguously looking symbols
	std::string getRandomBase58(size_t length);
	// even less ambiguosly looking symbols
	std::string getRandomWordSafeBase32(size_t length);
}

#pragma once

#ifdef BUILD_AUTO_TESTS

class ArgumentsParser;

namespace AutoTests
{
	// returns true if all tests passed
	bool RunTests(const ArgumentsParser& arguments);
}

#endif // BUILD_AUTO_TESTS

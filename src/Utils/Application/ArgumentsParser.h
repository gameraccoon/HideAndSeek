#pragma once

#include <string>
#include <vector>

class ArgumentsParser
{
public:
	ArgumentsParser(int argc, char **argv, const std::string& argumentSwitch = "--");

	[[nodiscard]] bool hasArgument(const std::string& argument) const;
	[[nodiscard]] std::string getArgumentValue(const std::string& argument, const std::string& defaultValue = "") const;
	[[nodiscard]] int getIntArgumentValue(const std::string& argument, int defaultValue = 0) const;

	const std::string& getArgumentSwitch() const { return mArgumentSwitch; }

private:
	std::vector<std::string> mTokens;
	const std::string mArgumentSwitch;
};

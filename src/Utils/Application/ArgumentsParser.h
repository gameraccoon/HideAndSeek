#pragma once

#include "Base/Types/ComplexTypes/Result.h"

#include <optional>
#include <string>
#include <vector>

class ArgumentsParser
{
public:
	ArgumentsParser(int argc, const char **argv, const std::string& argumentSwitch = "--");
	ArgumentsParser(int argc, char **argv, const std::string& argumentSwitch = "--");

	[[nodiscard]] bool hasArgument(const std::string& argument) const;
	[[nodiscard]] std::optional<std::string> getArgumentValue(const std::string& argument) const;
	[[nodiscard]] Result<int, std::string> getIntArgumentValue(const std::string& argument) const;

	const std::string& getArgumentSwitch() const { return mArgumentSwitch; }

private:
	std::vector<std::string> mTokens;
	const std::string mArgumentSwitch;
};

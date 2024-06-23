#pragma once

#include "Base/Types/ComplexTypes/Result.h"

#include <optional>
#include <string>
#include <vector>
#include <filesystem>

class ArgumentsParser
{
public:
	explicit ArgumentsParser(int argc, const char **argv, const std::string& argumentSwitch = "--");
	explicit ArgumentsParser(int argc, char **argv, const std::string& argumentSwitch = "--");

	[[nodiscard]] bool hasArgument(const std::string& argument) const;
	[[nodiscard]] std::optional<std::string> getArgumentValue(const std::string& argument) const;
	[[nodiscard]] Result<int, std::string> getIntArgumentValue(const std::string& argument) const;

	const std::string& getArgumentSwitch() const { return mArgumentSwitch; }

	[[nodiscard]] std::vector<std::string> getAllArguments() const;

	void manuallySetArgument(const std::string& argument, const std::string& value = "");

private:
	std::vector<std::string> mTokens;
	std::string mArgumentSwitch;
};

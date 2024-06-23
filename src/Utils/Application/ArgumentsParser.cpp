#include "Base/precomp.h"

#include "Utils/Application/ArgumentsParser.h"

#include "Base/Types/String/StringNumberConversion.h"

#include <format>

ArgumentsParser::ArgumentsParser(int argc, const char** argv, const std::string& argumentSwitch)
	: mArgumentSwitch(argumentSwitch)
{
	for (int i = 1; i < argc; ++i)
	{
		mTokens.emplace_back(argv[i]);
	}
}

ArgumentsParser::ArgumentsParser(int argc, char** argv, const std::string& argumentSwitch)
	: mArgumentSwitch(argumentSwitch)
{
	for (int i = 1; i < argc; ++i)
	{
		mTokens.emplace_back(argv[i]);
	}
}

std::optional<std::string> ArgumentsParser::getArgumentValue(const std::string& argument) const
{
	std::vector<std::string>::const_iterator itr = std::find(this->mTokens.begin(), this->mTokens.end(), mArgumentSwitch + argument);

	if (itr != mTokens.end())
	{
		++itr;

		if (itr != mTokens.end())
		{
			if (!itr->starts_with(mArgumentSwitch))
			{
				return *itr;
			}
		}
	}

	return std::nullopt;
}

Result<int, std::string> ArgumentsParser::getIntArgumentValue(const std::string& argument) const
{
	std::vector<std::string>::const_iterator itr = std::find(this->mTokens.begin(), this->mTokens.end(), mArgumentSwitch + argument);

	if (itr != mTokens.end())
	{
		++itr;
		if (itr != mTokens.end())
		{
			if (!itr->starts_with(mArgumentSwitch))
			{
				std::optional<int> result = String::ParseInt(itr->data(), 10);
				if (result.has_value())
				{
					return Result<int, std::string>::Ok(result.value());
				}
				else
				{
					return Result<int, std::string>::Err(std::format("Argument {} expected to have integer value, but the value is '{}'", argument.c_str(), itr->c_str()));
				}
			}
		}
	}

	return Result<int, std::string>::Err(std::format("No value was provided for argument {}", argument.c_str()));
}

bool ArgumentsParser::hasArgument(const std::string& argument) const
{
	return std::find(mTokens.begin(), mTokens.end(), mArgumentSwitch + argument) != mTokens.end();
}

std::vector<std::string> ArgumentsParser::getAllArguments() const
{
	std::vector<std::string> result;
	result.reserve(mTokens.size());

	for (const std::string& token : mTokens)
	{
		if (token.starts_with(mArgumentSwitch))
		{
			result.push_back(token.substr(mArgumentSwitch.size()));
		}
	}

	return result;
}

void ArgumentsParser::manuallySetArgument(const std::string& argument, const std::string& value)
{
	mTokens.emplace_back(mArgumentSwitch + argument);
	if (!value.empty())
	{
		mTokens.emplace_back(value);
	}
}

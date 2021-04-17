#include "Base/precomp.h"

#include "Utils/Application/ArgumentsParser.h"

#include <algorithm>

ArgumentsParser::ArgumentsParser(int argc, char** argv, const std::string& argumentSwitch)
	: mArgumentSwitch(argumentSwitch)
{
	for (int i = 1; i < argc; ++i)
	{
		mTokens.emplace_back(argv[i]);
	}
}

std::string ArgumentsParser::getArgumentValue(const std::string& argument, const std::string& defaultValue) const
{
	std::vector<std::string>::const_iterator itr = std::find(this->mTokens.begin(), this->mTokens.end(), mArgumentSwitch + argument);

	if (itr != this->mTokens.end())
	{
		++itr;
		if (itr != this->mTokens.end())
		{
			return *itr;
		}
	}

	return defaultValue;
}

bool ArgumentsParser::hasArgument(const std::string& argument) const
{
	return std::find(this->mTokens.begin(), this->mTokens.end(), mArgumentSwitch + argument) != this->mTokens.end();
}

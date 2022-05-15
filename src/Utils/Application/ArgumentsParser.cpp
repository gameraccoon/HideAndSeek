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

	if (itr != mTokens.end())
	{
		++itr;
		if (itr != mTokens.end())
		{
			return *itr;
		}
	}

	return defaultValue;
}

int ArgumentsParser::getIntArgumentValue(const std::string& argument, int defaultValue) const
{
	std::vector<std::string>::const_iterator itr = std::find(this->mTokens.begin(), this->mTokens.end(), mArgumentSwitch + argument);

	if (itr != mTokens.end())
	{
		++itr;
		if (itr != mTokens.end())
		{
			auto beginIt = itr->begin();
			if (itr->size() > 1u && *beginIt == '-')
			{
				++beginIt;
			}

			if (std::all_of(beginIt, itr->end(), [](char c){ return isdigit(c) != 0; }))
			{
				return std::atoi(itr->c_str());
			}
			else
			{
				ReportFatalError("Argument %s doesn't have integer value, the value is %s", argument.c_str(), itr->c_str());
			}
		}
	}

	return defaultValue;
}

bool ArgumentsParser::hasArgument(const std::string& argument) const
{
	return std::find(mTokens.begin(), mTokens.end(), mArgumentSwitch + argument) != mTokens.end();
}

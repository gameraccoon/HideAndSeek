#include "Base/precomp.h"

#include <iostream>

#include "Base/Random/Random.h"

#include "ECS/ErrorHandling.h"

#include "Utils/Application/ArgumentsParser.h"

#include "AutoTests/Tests/CollidingCircularUnits/TestCase.h"
#include "AutoTests/Tests/WeaponShooting/TestCase.h"
#include "AutoTests/TestChecklist.h"

using CasesMap = std::map<std::string, std::function<std::unique_ptr<BaseTestCase>()>>;

static CasesMap GetCases()
{
	return CasesMap
	({
		{
			"CollidingCircularUnits", []()
			{
				return std::make_unique<CollidingCircularUnitsTestCase>(800, 600);
			}
		},
		{
			"WeaponShooting", []()
			{
				return std::make_unique<WeaponShootingTestCase>(800, 600);
			}
		}
	});
}

bool ValidateChecklist(const TestChecklist& checklist)
{
	bool result = true;
	for(const auto& checkPair : checklist.checks)
	{
		if (!checkPair.second->isPassed())
		{
			LogInfo("Test check failed: %s. %s", checkPair.first, checkPair.second->describe());
			result = false;
			continue;
		}
	}
	return result;
}

int main(int argc, char** argv)
{
	ArgumentsParser arguments(argc, argv);

	unsigned int seed = 100500;
	if (arguments.hasArgument("randseed"))
	{
		std::string seedStr = arguments.getArgumentValue("randseed");
		seed = static_cast<unsigned int>(std::atoi(seedStr.c_str()));
	}

	Random::gGlobalGenerator = std::mt19937(seed);

#ifdef ECS_DEBUG_CHECKS_ENABLED
	Ecs::gErrorHandler = [](const std::string& error) { ReportFatalError(error); };
#endif // ECS_DEBUG_CHECKS_ENABLED

	auto cases = GetCases();

	if (arguments.hasArgument("list-cases"))
	{
		for (const auto& casePair : cases)
		{
			std::cout << casePair.first << "\n";
		}
		return 0;
	}

	if (!arguments.hasArgument("case"))
	{
		LogError("Test case name not provided");
		return 1;
	}

	auto caseIt = cases.find(arguments.getArgumentValue("case"));
	if (caseIt != cases.end())
	{
		std::unique_ptr<BaseTestCase> testCase = caseIt->second();
		TestChecklist checklist = testCase->start(arguments);
		bool isSuccessful = ValidateChecklist(checklist);
		return isSuccessful ? 0 : 1;
	}
	else
	{
		LogError("Unknown test " + arguments.getArgumentValue("case"));
		return 1;
	}
}

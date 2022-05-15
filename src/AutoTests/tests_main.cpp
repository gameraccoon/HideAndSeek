#include "Base/precomp.h"

#include <iostream>

#include "Base/Random/Random.h"

#include <raccoon-ecs/error_handling.h>

#include "Utils/Application/ArgumentsParser.h"

#include "HAL/Base/Engine.h"

#include "GameLogic/Game/ApplicationData.h"

#include "AutoTests/Tests/CollidingCircularUnits/TestCase.h"
#include "AutoTests/Tests/WeaponShooting/TestCase.h"
#include "AutoTests/TestChecklist.h"

using CasesMap = std::map<std::string, std::function<std::unique_ptr<BaseTestCase>(HAL::Engine*, ResourceManager&, ThreadPool&)>>;

static CasesMap GetCases()
{
	return CasesMap
	({
		{
			"CollidingCircularUnits", [](HAL::Engine* engine, ResourceManager& resourceManager, ThreadPool& threadPool)
			{
				return std::make_unique<CollidingCircularUnitsTestCase>(engine, resourceManager, threadPool);
			}
		},
		{
			"WeaponShooting", [](HAL::Engine* engine, ResourceManager& resourceManager, ThreadPool& threadPool)
			{
				return std::make_unique<WeaponShootingTestCase>(engine, resourceManager, threadPool);
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

#ifdef RACCOON_ECS_DEBUG_CHECKS_ENABLED
	RaccoonEcs::gErrorHandler = [](const std::string& error) { ReportFatalError(error); };
#endif // RACCOON_ECS_DEBUG_CHECKS_ENABLED

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
		ApplicationData applicationData(arguments.getIntArgumentValue("profile-systems", ApplicationData::DefaultWorkerThreadCount));
		HAL::Engine engine(800, 600);

		// switch render context to render thread
		engine.releaseRenderContext();
		applicationData.renderThread.startThread(applicationData.resourceManager, engine, [&engine]{ engine.acquireRenderContext(); });

		std::unique_ptr<BaseTestCase> testCase = caseIt->second(&engine, applicationData.resourceManager, applicationData.threadPool);
		TestChecklist checklist = testCase->start(arguments, applicationData.renderThread.getAccessor());
		bool isSuccessful = ValidateChecklist(checklist);

		applicationData.shutdownThreads(); // this call waits for the threads to be joined

		applicationData.writeProfilingData(); // this call waits for the data to be written to the files

		return isSuccessful ? 0 : 1;
	}
	else
	{
		LogError("Unknown test " + arguments.getArgumentValue("case"));
		return 1;
	}
}

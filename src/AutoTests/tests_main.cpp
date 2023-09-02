#include "Base/precomp.h"

#include <iostream>

#include "Base/Random/Random.h"
#include "Base/Types/String/StringNumberConversion.h"

#include <raccoon-ecs/error_handling.h>

#include "Utils/Application/ArgumentsParser.h"

#include "HAL/Base/Engine.h"

#include "GameLogic/Game/ApplicationData.h"

#include "AutoTests/BaseTestCase.h"
#include "AutoTests/TestChecks.h"

#include "AutoTests/Tests/CollidingCircularUnits/TestCase.h"
#include "AutoTests/Tests/WeaponShooting/TestCase.h"

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
	size_t failedChecksCount = 0;
	for(const auto& check : checklist.checks)
	{
		if (!check->hasPassed())
		{
			if (check->wasChecked())
			{
				LogInfo("Test check failed: %s", check->getErrorMessage());
			}
			else
			{
				LogInfo("Test check was not validated, assume failed: %s", check->getErrorMessage());
			}
			++failedChecksCount;
		}
	}

	if (failedChecksCount > 0)
	{
		LogInfo("Failed %u checks out of %u", failedChecksCount, checklist.checks.size());
		return false;
	}
	else
	{
		LogInfo("Passed %d checks out of %d", checklist.checks.size(), checklist.checks.size());
		return true;
	}
}

int main(int argc, char** argv)
{
	ArgumentsParser arguments(argc, argv);

	unsigned int seed = std::random_device()();
	if (arguments.hasArgument("randseed"))
	{
		std::string seedStr = arguments.getArgumentValue("randseed");
		if (std::optional<int> seedOpt = String::ParseInt(seedStr.c_str()); seedOpt.has_value())
		{
			seed = static_cast<unsigned int>(seedOpt.value());
		}
		else
		{
			std::cout << "Invalid random seed value: " << seedStr << "\n";
			return 1;
		}
	}

	Random::gGlobalGenerator = std::mt19937(seed);

#ifdef RACCOON_ECS_DEBUG_CHECKS_ENABLED
	RaccoonEcs::gErrorHandler = [](const std::string& error) { ReportFatalError(error); };
#endif // RACCOON_ECS_DEBUG_CHECKS_ENABLED

	gGlobalAssertHandler = []{ std::terminate(); };

	auto cases = GetCases();

	if (arguments.hasArgument("list"))
	{
		for (const auto& casePair : cases)
		{
			std::cout << casePair.first << "\n";
		}
		return 0;
	}

	if (!arguments.hasArgument("case"))
	{
		std::cout << "Test case name has not been provided, use:\n\t--list to get the list of available test cases\n\t--case <name> to run a specific test case\n";
		return 1;
	}

	auto caseIt = cases.find(arguments.getArgumentValue("case"));
	if (caseIt != cases.end())
	{
		LogInit("Random seed is %u", seed);

		ApplicationData applicationData(arguments.getIntArgumentValue("threads-count", ApplicationData::DefaultWorkerThreadCount));
		HAL::Engine engine(800, 600);

		// switch render context to render thread
		engine.releaseRenderContext();
		applicationData.renderThread.startThread(applicationData.resourceManager, engine, [&engine]{ engine.acquireRenderContext(); });

		std::unique_ptr<BaseTestCase> testCase = caseIt->second(&engine, applicationData.resourceManager, applicationData.threadPool);
		TestChecklist checklist = testCase->start(arguments, RenderAccessorGameRef(applicationData.renderThread.getAccessor(), 0));
		bool isSuccessful = ValidateChecklist(checklist);

		applicationData.shutdownThreads(); // this call waits for the threads to be joined

		applicationData.writeProfilingData(); // this call waits for the data to be written to the files

		std::cout << "Test run " << (isSuccessful ? "was successful" : "failed, see the full log for errors") << "\n";
		return isSuccessful ? 0 : 1;
	}
	else
	{
		std::cout << "Unknown test '" << arguments.getArgumentValue("case") << "'\n";
		return 1;
	}
}

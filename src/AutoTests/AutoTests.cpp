#include "EngineCommon/precomp.h"

#include <iostream>

#include <raccoon-ecs/error_handling.h>

#include "EngineCommon/Random/Random.h"

#include "HAL/Base/Engine.h"

#include "EngineUtils/Application/ArgumentsParser.h"

#include "GameLogic/Game/ApplicationData.h"

#include "AutoTests/BaseTestCase.h"
#include "AutoTests/TestCheckList.h"
#include "AutoTests/Tests/CollidingCircularUnits/TestCase.h"
#include "AutoTests/Tests/WeaponShooting/TestCase.h"

namespace AutoTests
{
	using CasesMap = std::map<std::string, std::function<std::unique_ptr<BaseTestCase>(HAL::Engine*, ResourceManager&, ThreadPool&)>>;

	static CasesMap GetCases()
	{
		CasesMap cases;

		cases.emplace("CollidingCircularUnits", [](HAL::Engine* engine, ResourceManager& resourceManager, ThreadPool& threadPool) {
			return std::make_unique<CollidingCircularUnitsTestCase>(engine, resourceManager, threadPool);
		});
		cases.emplace("WeaponShooting", [](HAL::Engine* engine, ResourceManager& resourceManager, ThreadPool& threadPool) {
			return std::make_unique<WeaponShootingTestCase>(engine, resourceManager, threadPool);
		});

		return cases;
	}

	static bool ValidateChecklist(const TestChecklist& checklist)
	{
		size_t failedChecksCount = 0;
		for (const auto& check : checklist.getChecks())
		{
			if (!check->hasPassed())
			{
				if (check->hasBeenValidated())
				{
					LogInfo("Test check failed: %s", check->getErrorMessage());
				}
				else
				{
					LogInfo("Test check was not validated, assume failed: %s", check->getErrorMessage());
				}
				++failedChecksCount;
			}
			else
			{
				Assert(check->hasBeenValidated(), "Test check has passed but was not validated. This looks like a logical error in the test code.");
			}
		}

		const size_t totalChecksCount = checklist.getChecks().size();

		if (failedChecksCount > 0)
		{
			LogInfo("Failed %u checks out of %u", failedChecksCount, totalChecksCount);
			return false;
		}
		else
		{
			LogInfo("Passed %d checks out of %d", totalChecksCount, totalChecksCount);
			return true;
		}
	}

	bool RunTests(const ArgumentsParser& arguments)
	{
		unsigned int seed = std::random_device()();
		if (arguments.hasArgument("randseed"))
		{
			const auto seedValue = arguments.getIntArgumentValue("randseed");
			if (seedValue.hasValue())
			{
				seed = static_cast<unsigned int>(seedValue.getValue());
			}
			else
			{
				std::cout << seedValue.getError() << "\n";
				return false;
			}
		}

		const bool hasRender = !arguments.hasArgument("no-render");

		Random::gGlobalGenerator = std::mt19937(seed);

#ifdef RACCOON_ECS_DEBUG_CHECKS_ENABLED
		RaccoonEcs::gErrorHandler = [](const std::string& error) { ReportFatalError(error); };
#endif // RACCOON_ECS_DEBUG_CHECKS_ENABLED

		gGlobalAssertHandler = [] { std::terminate(); };

		auto cases = GetCases();

		if (arguments.hasArgument("list"))
		{
			for (const auto& casePair : cases)
			{
				std::cout << casePair.first << "\n";
			}
			return true;
		}

		if (!arguments.hasArgument("case"))
		{
			std::cout << "Test case name has not been provided, use:\n\t--list to get the list of available test cases\n\t--case <name> to run a specific test case\n";
			return false;
		}

		const auto caseIt = cases.find(arguments.getArgumentValue("case").value_or(""));
		if (caseIt == cases.end())
		{
			std::cout << "Unknown test '" << arguments.getArgumentValue("case").value_or("") << "'\n";
			return false;
		}
		LogInit("Random seed is %u", seed);

		ApplicationData applicationData(arguments.getIntArgumentValue("threads-count").getValueOr(ApplicationData::DefaultWorkerThreadCount));

		std::unique_ptr<HAL::Engine> engine;
		if (hasRender)
		{
			engine = std::make_unique<HAL::Engine>(800, 600);

			// switch render context to render thread
			engine->releaseRenderContext();
			applicationData.renderThread.startThread(applicationData.resourceManager, *engine, [&engine] { engine->acquireRenderContext(); });
		}
		const std::unique_ptr<BaseTestCase> testCase = caseIt->second(engine.get(), applicationData.resourceManager, applicationData.threadPool);
		const TestChecklist checklist = testCase->start(arguments, RenderAccessorGameRef(applicationData.renderThread.getAccessor(), 0));
		const bool isSuccessful = ValidateChecklist(checklist);

		applicationData.shutdownThreads(); // this call waits for the threads to be joined

		ApplicationData::writeProfilingData(); // this call waits for the data to be written to the files

		std::cout << "Test run " << (isSuccessful ? "was successful" : "failed, see the full log for errors") << "\n";
		return isSuccessful;
	}
} // namespace AutoTests

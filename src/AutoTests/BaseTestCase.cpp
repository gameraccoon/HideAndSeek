#include "Base/precomp.h"

#include "AutoTests/BaseTestCase.h"

#include <memory>

#include "Base/Types/TemplateHelpers.h"

#include "HAL/Base/Engine.h"

#include "GameData/ComponentRegistration/ComponentFactoryRegistration.h"

BaseTestCase::BaseTestCase(int width, int height)
	: HAL::GameBase(width, height)
{
}

TestChecklist BaseTestCase::start(const ArgumentsParser& arguments)
{
	mOneFrame = arguments.hasArgument("one-frame");

	mProfileSystems = arguments.hasArgument("profile-systems");
	mSystemProfileOutputPath = arguments.getArgumentValue("profile-systems", mSystemProfileOutputPath);

	ComponentsRegistration::RegisterComponents(mComponentFactory);

	initTestCase(arguments);


	getEngine().releaseRenderContext();
	mRenderThread.startThread(getResourceManager(), getEngine(), [&engine = getEngine()]{ engine.acquireRenderContext(); });

	// start the main loop
	getEngine().start(this);

	return std::move(mTestChecklist);
}

void BaseTestCase::update(float)
{
	constexpr float fixedDt = 1.0f / 60.0f;

	std::unique_ptr<RenderData> renderCommands = std::make_unique<RenderData>();
	TemplateHelpers::EmplaceVariant<SwapBuffersCommand>(renderCommands->layers);
	mRenderThread.getAccessor().submitData(std::move(renderCommands));

	do
	{
		mTime.update(fixedDt);
		mSystemsManager.update();
		++mTicksCount;
#ifdef RACCOON_ECS_PROFILE_SYSTEMS
		if (mProfileSystems)
		{
			mSystemFrameRecords.addFrame(mSystemsManager.getPreviousFrameTimeData());
		}
#endif // RACCOON_ECS_PROFILE_SYSTEMS
	}
	while (mOneFrame && mTicksCount < mTicksToFinish);

	if (mTicksCount >= mTicksToFinish)
	{
		finalizeTestCase();
		getEngine().quit();
	}
}

void BaseTestCase::initResources()
{
	getResourceManager().loadAtlasesData("resources/atlas/atlas-list.json");
	mSystemsManager.initResources();
}

void BaseTestCase::finalizeTestCase()
{
	if (mProfileSystems)
	{
//		mSystemFrameRecords.printToFile(mSystemsManager.getSystemNames(), mSystemProfileOutputPath);
	}
}

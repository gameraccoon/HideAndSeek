#include "Base/precomp.h"

#include "AutoTests/BaseTestCase.h"

#include <memory>

#include "Base/Types/TemplateHelpers.h"

#include "HAL/Base/Engine.h"

#include "GameData/ComponentRegistration/ComponentFactoryRegistration.h"
#include "GameData/ComponentRegistration/ComponentJsonSerializerRegistration.h"

TestChecklist BaseTestCase::start(const ArgumentsParser& arguments)
{
	mOneFrame = arguments.hasArgument("one-frame");

	ComponentsRegistration::RegisterComponents(getComponentFactory());
	ComponentsRegistration::RegisterJsonSerializers(getComponentSerializers());

	initTestCase(arguments);

	Game::preStart(arguments, 3);
	Game::start();
	Game::onGameShutdown();

	return std::move(mTestChecklist);
}

void BaseTestCase::fixedTimeUpdate(float dt)
{
	do
	{
		Game::fixedTimeUpdate(dt);
		++mTicksCount;
	}
	while (mOneFrame && mTicksCount < mTicksToFinish);

	if (mTicksCount >= mTicksToFinish)
	{
		finalizeTestCase();
		getEngine()->quit();
	}
}

void BaseTestCase::finalizeTestCase()
{
}

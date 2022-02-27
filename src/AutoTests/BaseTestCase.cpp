#include "Base/precomp.h"

#include "AutoTests/BaseTestCase.h"

#include <memory>

#include "Base/Types/TemplateHelpers.h"

#include "HAL/Base/Engine.h"

#include "GameData/ComponentRegistration/ComponentFactoryRegistration.h"
#include "GameData/ComponentRegistration/ComponentJsonSerializerRegistration.h"

BaseTestCase::BaseTestCase(int width, int height)
	: Game(width, height)
{
}

TestChecklist BaseTestCase::start(const ArgumentsParser& arguments)
{
	mOneFrame = arguments.hasArgument("one-frame");

	initTestCase(arguments);

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
		getEngine().quit();
	}
}

void BaseTestCase::startGame(const ArgumentsParser& arguments)
{
	ComponentsRegistration::RegisterComponents(getComponentFactory());
	ComponentsRegistration::RegisterJsonSerializers(getComponentSerializers());

	Game::start(arguments, 3);
}

void BaseTestCase::finalizeTestCase()
{
}

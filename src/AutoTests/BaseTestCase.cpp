#include "Base/precomp.h"

#include "AutoTests/BaseTestCase.h"

#include "GameData/ComponentRegistration/ComponentFactoryRegistration.h"
#include "GameData/ComponentRegistration/ComponentJsonSerializerRegistration.h"
#include "GameData/Components/RenderAccessorComponent.generated.h"

#include "Utils/Application/ArgumentsParser.h"

#include "HAL/Base/Engine.h"
#include "HAL/Base/GameLoop.h"

TestChecklist BaseTestCase::start(const ArgumentsParser& arguments, RenderAccessorGameRef renderAccessor)
{
	mOneFrame = arguments.hasArgument("one-frame");

	ComponentsRegistration::RegisterComponents(getComponentFactory());
	ComponentsRegistration::RegisterJsonSerializers(getComponentSerializers());

	const bool hasRender = !arguments.hasArgument("no-render");

	if (hasRender)
	{
		RenderAccessorComponent* renderAccessorComponent = getGameData().getGameComponents().getOrAddComponent<RenderAccessorComponent>();
		renderAccessorComponent->setAccessor(renderAccessor);
	}

	initTestCase(arguments);

	Game::initResources();
	Game::preStart(arguments);
	if (hasRender)
	{
		getEngine()->init(this, nullptr);
		getEngine()->start();
	}
	else
	{
		HAL::RunGameLoop(*this, nullptr, nullptr);
	}
	Game::onGameShutdown();

	return std::move(mTestChecklist);
}

void BaseTestCase::fixedTimeUpdate(const float dt)
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
		mShouldQuit = true;
	}
}

void BaseTestCase::finalizeTestCase()
{
}

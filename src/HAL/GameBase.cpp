#include "Base/precomp.h"

#include "HAL/GameBase.h"

#include "Utils/ResourceManagement/ResourceManager.h"

namespace HAL
{
	GameBase::GameBase(Engine* engine, ResourceManager& resourceManager)
		: mEngine(engine)
		, mResourceManager(resourceManager)
	{
	}

	GameBase::~GameBase() = default;

	Engine* GameBase::getEngine()
	{
		return mEngine;
	}

	ResourceManager& GameBase::getResourceManager()
	{
		return mResourceManager;
	}
}

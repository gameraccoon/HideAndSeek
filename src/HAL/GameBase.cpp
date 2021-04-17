#include "Base/precomp.h"

#include "HAL/GameBase.h"

#include "HAL/Base/Engine.h"

namespace HAL
{
	GameBase::GameBase(int windowWidth, int windowHeight)
		: mEngine(HS_NEW Engine(windowWidth, windowHeight))
		, mResourceManager(HS_NEW ResourceManager())
	{
	}

	GameBase::~GameBase() = default;

	Engine& GameBase::getEngine()
	{
		return *mEngine;
	}

	ResourceManager& GameBase::getResourceManager()
	{
		return *mResourceManager;
	}
}

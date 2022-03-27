#pragma once

#include "IGame.h"

#include "HAL/EngineFwd.h"

class ResourceManager;

namespace HAL
{
	class GameBase : public IGame
	{
	public:
		GameBase(Engine* engine, ResourceManager& resourceManager);
		~GameBase() override;

	protected:
		Engine* getEngine();
		ResourceManager& getResourceManager();

	private:
		Engine* mEngine;
		ResourceManager& mResourceManager;
	};
}

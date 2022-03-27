#pragma once

#include "IGame.h"

#include "HAL/EngineFwd.h"

#include "Utils/ResourceManagement/ResourceManager.h"

namespace HAL
{
	class GameBase : public IGame
	{
	public:
		GameBase(int windowWidth, int windowHeight);
		~GameBase() override;

	protected:
		Engine* getEngine();
		ResourceManager& getResourceManager();

	private:
		std::unique_ptr<Engine> mEngine;
		std::unique_ptr<ResourceManager> mResourceManager;
	};
}

#pragma once

#include "Game.h"

#include <Core/World.h>
#include <Graphics/Camera.h>

namespace Game
{
	class StealthGame : public SystemInterface::Game
	{
	public:
		StealthGame() = default;

		void start();
		virtual void update(float dt) override;
		virtual void draw() override;

	private:
		std::unique_ptr<World> mWorld;
		std::unique_ptr<Camera> mMainCamera;
	};
}
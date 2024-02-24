#pragma once

#ifndef DISABLE_SDL

#include <raccoon-ecs/utils/system.h>

class WorldHolder;

namespace HAL
{
	class InputControllersData;
}

/**
 * System that processes debug input
 */
class DebugInputSystem : public RaccoonEcs::System
{
public:
	DebugInputSystem(
		WorldHolder& worldHolder,
		const HAL::InputControllersData& inputData,
		bool& shouldPauseGame
	) noexcept;

	void update() override;

private:
	void processDebugInput();

private:
	WorldHolder& mWorldHolder;
	const HAL::InputControllersData& mInputData;
	bool& mShouldPauseGame;
};

#endif // !DISABLE_SDL

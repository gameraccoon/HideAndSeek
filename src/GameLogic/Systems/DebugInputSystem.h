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
class DebugInputSystem final : public RaccoonEcs::System
{
public:
	explicit DebugInputSystem(
		WorldHolder& worldHolder,
		const HAL::InputControllersData& inputData,
		bool& shouldPauseGame
	) noexcept;

	void update() override;

private:
	void processDebugInput() const;

private:
	WorldHolder& mWorldHolder;
	const HAL::InputControllersData& mInputData;
	bool& mShouldPauseGame;
};

#endif // !DISABLE_SDL

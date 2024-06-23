#pragma once

#ifndef DISABLE_SDL

#include <raccoon-ecs/utils/system.h>

class WorldHolder;

namespace HAL
{
	class InputControllersData;
}

/**
 * System that transforms raw controller input into gameplay input commands
 */
class InputSystem final : public RaccoonEcs::System
{
public:
	explicit InputSystem(
		WorldHolder& worldHolder,
		const HAL::InputControllersData& inputData
	) noexcept;

	void update() override;

private:
	void processGameplayInput() const;

private:
	WorldHolder& mWorldHolder;
	const HAL::InputControllersData& mInputData;
};

#endif // !DISABLE_SDL

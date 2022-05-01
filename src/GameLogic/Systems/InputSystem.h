#pragma once

#include <raccoon-ecs/system.h>

class WorldHolder;
class TimeData;

namespace HAL
{
	class InputControllersData;
}

/**
 * System that transforms raw controller input into gameplay input commands
 */
class InputSystem : public RaccoonEcs::System
{
public:
	InputSystem(WorldHolder& worldHolder, const HAL::InputControllersData& inputData, const TimeData& timeData) noexcept;

	void update() override;
	static std::string GetSystemId() { return "InputSystem"; }

private:
	void processGameplayInput();
	void processDebugInput();

private:
	WorldHolder& mWorldHolder;
	const HAL::InputControllersData& mInputData;
	const TimeData& mTime;
};

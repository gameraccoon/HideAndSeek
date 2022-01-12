#pragma once

#include <unordered_map>

#include <raccoon-ecs/system.h>

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/InputData.h"

/**
 * System that handles camera position
 */
class CameraSystem : public RaccoonEcs::System
{
public:
	CameraSystem(
		WorldHolder& worldHolder,
		const InputData& inputData) noexcept;
	~CameraSystem() override = default;

	void update() override;
	static std::string GetSystemId() { return "CameraSystem"; }

private:
	WorldHolder& mWorldHolder;
	const InputData& mInputData;
};

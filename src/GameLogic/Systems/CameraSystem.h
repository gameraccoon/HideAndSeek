#pragma once

#include <unordered_map>

#include "ECS/System.h"

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/InputData.h"

/**
 * System that handles camera position
 */
class CameraSystem : public Ecs::System
{
public:
	CameraSystem(WorldHolder& worldHolder, const InputData& inputData) noexcept;
	~CameraSystem() override = default;

	void update() override;
	std::string getName() override { return "CameraSystem"; }

private:
	WorldHolder& mWorldHolder;
	const InputData& mInputData;
};

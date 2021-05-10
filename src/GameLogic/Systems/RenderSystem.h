#pragma once

#include <memory>
#include <vector>

#include <raccoon-ecs/system.h>

#include "Utils/Jobs/WorkerManager.h"

#include "HAL/Base/ResourceManager.h"
#include "HAL/EngineFwd.h"

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

struct Vector2D;

/**
 * System that handles rendering of world objects
 */
class RenderSystem : public RaccoonEcs::System
{
public:
	RenderSystem(WorldHolder& worldHolder,
		const TimeData& timeData,
		HAL::Engine& engine,
		HAL::ResourceManager& resourceManager,
		Jobs::WorkerManager& jobsWorkerManager) noexcept;

	~RenderSystem() override = default;

	void update() override;
	std::string getName() override { return "RenderSystem"; }

private:
	static void DrawVisibilityPolygon(const Graphics::Sprite& lightSprite, const std::vector<Vector2D>& polygon, const Vector2D& fowSize, const Vector2D& drawShift);
	void drawBackground(World& world, const Vector2D& drawShift);
	static Vector2D GetPlayerSightPosition(World& world);
	void drawLights(class SpatialEntityManager& managerGroup, std::vector<class WorldCell*>& cells, Vector2D playerSightPosition, Vector2D drawShift, Vector2D maxFov, Vector2D screenHalfSize);

private:
	WorldHolder& mWorldHolder;
	const TimeData& mTime;
	HAL::Engine& mEngine;
	HAL::ResourceManager& mResourceManager;
	Jobs::WorkerManager& mJobsWorkerManager;
	ResourceHandle mLightSpriteHandle;
};

#pragma once

#include <memory>
#include <vector>

#include <raccoon-ecs/system.h>
#include <raccoon-ecs/async_operations.h>

#include "Utils/Jobs/WorkerManager.h"

#include "HAL/Base/ResourceManager.h"
#include "HAL/EngineFwd.h"

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

struct Vector2D;
class WorldCachedDataComponent;
class RenderModeComponent;
class BackgroundTextureComponent;
class LightBlockingGeometryComponent;
class RenderComponent;
class TransformComponent;

/**
 * System that handles rendering of world objects
 */
class RenderSystem : public RaccoonEcs::System
{
public:
	RenderSystem(
		RaccoonEcs::ComponentFilter<const WorldCachedDataComponent>&& worldCachedDataFilter,
		RaccoonEcs::ComponentFilter<const RenderModeComponent>&& renderModeFilter,
		RaccoonEcs::ComponentFilter<BackgroundTextureComponent>&& backgroundTextureFilter,
		RaccoonEcs::ComponentFilter<const LightBlockingGeometryComponent>&& lightBlockingGeometryFilter,
		RaccoonEcs::ComponentFilter<const RenderComponent, const TransformComponent>&& renderFilter,
		WorldHolder& worldHolder,
		const TimeData& timeData,
		HAL::Engine& engine,
		HAL::ResourceManager& resourceManager,
		Jobs::WorkerManager& jobsWorkerManager) noexcept;

	~RenderSystem() override = default;

	void update() override;
	std::string getName() const override { return "RenderSystem"; }

private:
	static void DrawVisibilityPolygon(const Graphics::Sprite& lightSprite, const std::vector<Vector2D>& polygon, const Vector2D& fowSize, const Vector2D& drawShift);
	void drawBackground(World& world, const Vector2D& drawShift);
	void drawLights(class SpatialEntityManager& managerGroup, std::vector<class WorldCell*>& cells, Vector2D playerSightPosition, Vector2D drawShift, Vector2D maxFov, Vector2D screenHalfSize);

private:
	RaccoonEcs::ComponentFilter<const WorldCachedDataComponent> mWorldCachedDataFilter;
	RaccoonEcs::ComponentFilter<const RenderModeComponent> mRenderModeFilter;
	RaccoonEcs::ComponentFilter<BackgroundTextureComponent> mBackgroundTextureFilter;
	RaccoonEcs::ComponentFilter<const LightBlockingGeometryComponent> mLightBlockingGeometryFilter;
	RaccoonEcs::ComponentFilter<const RenderComponent, const TransformComponent> mRenderFilter;
	WorldHolder& mWorldHolder;
	const TimeData& mTime;
	HAL::Engine& mEngine;
	HAL::ResourceManager& mResourceManager;
	Jobs::WorkerManager& mJobsWorkerManager;
	ResourceHandle mLightSpriteHandle;
};

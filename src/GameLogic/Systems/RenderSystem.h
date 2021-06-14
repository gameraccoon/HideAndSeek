#pragma once

#include <memory>
#include <vector>

#include <raccoon-ecs/system.h>
#include <raccoon-ecs/async_operations.h>

#include "GameData/Components/RenderComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/LightComponent.generated.h"
#include "GameData/Components/LightBlockingGeometryComponent.generated.h"
#include "GameData/Components/RenderModeComponent.generated.h"
#include "GameData/Components/WorldCachedDataComponent.generated.h"
#include "GameData/Components/BackgroundTextureComponent.generated.h"

#include "Utils/Jobs/WorkerManager.h"

#include "HAL/Base/ResourceManager.h"
#include "HAL/EngineFwd.h"

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

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
		RaccoonEcs::ComponentFilter<LightComponent, const TransformComponent>&& lightFilter,
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
	RaccoonEcs::ComponentFilter<LightComponent, const TransformComponent> mLightFilter;
	WorldHolder& mWorldHolder;
	const TimeData& mTime;
	HAL::Engine& mEngine;
	HAL::ResourceManager& mResourceManager;
	Jobs::WorkerManager& mJobsWorkerManager;
	ResourceHandle mLightSpriteHandle;
};

#pragma once

#include <memory>
#include <vector>

#include <raccoon-ecs/system.h>
#include <raccoon-ecs/async_operations.h>
#include <raccoon-ecs/thread_pool.h>

#include "GameData/Components/SpriteRenderComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/LightComponent.generated.h"
#include "GameData/Components/LightBlockingGeometryComponent.generated.h"
#include "GameData/Components/RenderModeComponent.generated.h"
#include "GameData/Components/WorldCachedDataComponent.generated.h"
#include "GameData/Components/BackgroundTextureComponent.generated.h"
#include "GameData/Components/RenderAccessorComponent.generated.h"

#include "HAL/Base/ResourceManager.h"

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

struct RenderData;

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
		RaccoonEcs::ComponentFilter<const SpriteRenderComponent, const TransformComponent>&& spriteRenderFilter,
		RaccoonEcs::ComponentFilter<LightComponent, const TransformComponent>&& lightFilter,
		RaccoonEcs::ComponentFilter<RenderAccessorComponent>&& renderAccessorFilter,
		WorldHolder& worldHolder,
		const TimeData& timeData,
		HAL::ResourceManager& resourceManager,
		RaccoonEcs::ThreadPool& threadPool) noexcept;

	~RenderSystem() override = default;

	void update() override;
	static std::string GetSystemId() { return "RenderSystem"; }

private:
	static void DrawVisibilityPolygon(RenderData& renderData, ResourceHandle lightSpriteHandle, const std::vector<Vector2D>& polygon, const Vector2D& fovSize, const Vector2D& drawShift);
	void drawBackground(RenderData& renderData, World& world, Vector2D drawShift, Vector2D windowSize);
	void drawLights(RenderData& renderData, class SpatialEntityManager& managerGroup, std::vector<class WorldCell*>& cells, Vector2D playerSightPosition, Vector2D drawShift, Vector2D maxFov, Vector2D screenHalfSize);

private:
	RaccoonEcs::ComponentFilter<const WorldCachedDataComponent> mWorldCachedDataFilter;
	RaccoonEcs::ComponentFilter<const RenderModeComponent> mRenderModeFilter;
	RaccoonEcs::ComponentFilter<BackgroundTextureComponent> mBackgroundTextureFilter;
	RaccoonEcs::ComponentFilter<const LightBlockingGeometryComponent> mLightBlockingGeometryFilter;
	RaccoonEcs::ComponentFilter<const SpriteRenderComponent, const TransformComponent> mSpriteRenderFilter;
	RaccoonEcs::ComponentFilter<LightComponent, const TransformComponent> mLightFilter;
	RaccoonEcs::ComponentFilter<RenderAccessorComponent> mRenderAccessorFilter;
	WorldHolder& mWorldHolder;
	const TimeData& mTime;
	HAL::ResourceManager& mResourceManager;
	RaccoonEcs::ThreadPool& mThreadPool;
	ResourceHandle mLightSpriteHandle;
};

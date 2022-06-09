#pragma once

#include <memory>
#include <vector>

#include <raccoon-ecs/system.h>

#include "Utils/ResourceManagement/ResourceManager.h"
#include "Utils/Multithreading/ThreadPool.h"

#include "GameLogic/SharedManagers/WorldHolder.h"

struct RenderData;
class GameplayTimestamp;

/**
 * System that handles rendering of world objects
 */
class RenderSystem : public RaccoonEcs::System
{
public:
	RenderSystem(
		WorldHolder& worldHolder,
		ResourceManager& resourceManager,
		ThreadPool& threadPool) noexcept;

	~RenderSystem() override = default;

	void update() override;
	static std::string GetSystemId() { return "RenderSystem"; }

private:
	static void DrawVisibilityPolygon(RenderData& renderData, ResourceHandle lightSpriteHandle, const std::vector<Vector2D>& polygon, const Vector2D& fovSize, const Vector2D& drawShift);
	void drawBackground(RenderData& renderData, World& world, Vector2D drawShift, Vector2D windowSize);
	void drawLights(RenderData& renderData, class SpatialEntityManager& managerGroup, std::vector<class WorldCell*>& cells, Vector2D playerSightPosition, Vector2D drawShift, Vector2D maxFov, Vector2D screenHalfSize, const GameplayTimestamp& timestampNow);

private:
	WorldHolder& mWorldHolder;
	ResourceManager& mResourceManager;
	ThreadPool& mThreadPool;
	ResourceHandle mLightSpriteHandle;
};

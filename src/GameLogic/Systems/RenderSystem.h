#pragma once

#include <vector>

#include <raccoon-ecs/system.h>

#include "GameData/Geometry/Vector2D.h"
#include "GameData/Resources/ResourceHandle.h"

class GameplayTimestamp;
class ResourceManager;
class ThreadPool;
class World;
class WorldHolder;
struct RenderData;

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

	void update() override;

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

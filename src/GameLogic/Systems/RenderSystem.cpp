#include "Base/precomp.h"

#include "GameLogic/Systems/RenderSystem.h"

#include <algorithm>

#include "Base/Types/TemplateAliases.h"
#include "Base/Types/TemplateHelpers.h"

#include "GameData/Components/BackgroundTextureComponent.generated.h"
#include "GameData/Components/LightBlockingGeometryComponent.generated.h"
#include "GameData/Components/LightComponent.generated.h"
#include "GameData/Components/RenderAccessorComponent.generated.h"
#include "GameData/Components/RenderModeComponent.generated.h"
#include "GameData/Components/SpriteRenderComponent.generated.h"
#include "GameData/Components/TimeComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/WorldCachedDataComponent.generated.h"
#include "GameData/GameData.h"
#include "GameData/World.h"

#include "Utils/Geometry/VisibilityPolygon.h"
#include "Utils/Multithreading/ThreadPool.h"
#include "Utils/ResourceManagement/ResourceManager.h"

#include "HAL/Graphics/Sprite.h"

#include "GameLogic/Render/RenderAccessor.h"
#include "GameLogic/SharedManagers/WorldHolder.h"

RenderSystem::RenderSystem(
		WorldHolder& worldHolder,
		ResourceManager& resourceManager,
		ThreadPool& threadPool
	) noexcept
	: mWorldHolder(worldHolder)
	, mResourceManager(resourceManager)
	, mThreadPool(threadPool)
{
	mLightSpriteHandle = resourceManager.lockResource<Graphics::Sprite>(RelativeResourcePath("resources/textures/light.png"));
}

void RenderSystem::update()
{
	SCOPED_PROFILER("RenderSystem::update");
	World& world = mWorldHolder.getWorld();
	GameData& gameData = mWorldHolder.getGameData();

	const auto [worldCachedData] = world.getWorldComponents().getComponents<WorldCachedDataComponent>();
	Vector2D workingRect = worldCachedData->getScreenSize();
	Vector2D cameraLocation = worldCachedData->getCameraPos();

	static const Vector2D maxFov(500.0f, 500.0f);

	const auto [renderMode] = gameData.getGameComponents().getComponents<RenderModeComponent>();
	const auto [time] = world.getWorldComponents().getComponents<TimeComponent>();

	Vector2D halfWindowSize = workingRect * 0.5f;

	Vector2D drawShift = halfWindowSize - cameraLocation;

	SpatialEntityManager::RecordsVector cells = world.getSpatialData().getCellsAround(cameraLocation, workingRect);
	SpatialEntityManager spatialManager(cells);

	auto [renderAccessorCmp] = gameData.getGameComponents().getComponents<RenderAccessorComponent>();
	if (renderAccessorCmp == nullptr || !renderAccessorCmp->getAccessor().has_value())
	{
		return;
	}

	RenderAccessorGameRef renderAccessor = *renderAccessorCmp->getAccessor();

	std::unique_ptr<RenderData> renderData = std::make_unique<RenderData>();

	if (!renderMode || renderMode->getIsDrawBackgroundEnabled())
	{
		drawBackground(*renderData, world, drawShift, workingRect);
	}

	if (!renderMode || renderMode->getIsDrawLightsEnabled())
	{
		const GameplayTimestamp timestampNow = time->getValue().lastFixedUpdateTimestamp;
		drawLights(*renderData, spatialManager, cells, cameraLocation, drawShift, maxFov, halfWindowSize, timestampNow);
	}

	if (!renderMode || renderMode->getIsDrawVisibleEntitiesEnabled())
	{
		SCOPED_PROFILER("draw visible entities");
		spatialManager.forEachComponentSet<const SpriteRenderComponent, const TransformComponent>(
			[&drawShift, &renderData](const SpriteRenderComponent* spriteRender, const TransformComponent* transform)
		{
			Vector2D location = transform->getLocation() + drawShift;
			float rotation = transform->getRotation().getValue();
			for (const auto& data : spriteRender->getSpriteDatas())
			{
				QuadRenderData& quadData = TemplateHelpers::EmplaceVariant<QuadRenderData>(renderData->layers);
				quadData.spriteHandle = data.spriteHandle;
				quadData.position = location;
				quadData.size = data.params.size;
				quadData.anchor = data.params.anchor;
				quadData.rotation = rotation;
				quadData.alpha = 1.0f;
			}
		});
	}

	renderAccessor.submitData(std::move(renderData));
}

void RenderSystem::DrawVisibilityPolygon(RenderData& renderData, ResourceHandle lightSpriteHandle, const std::vector<Vector2D>& polygon, const Vector2D& fovSize, const Vector2D& drawShift)
{
	if (polygon.size() > 2)
	{
		FanRenderData& lightPolygon = TemplateHelpers::EmplaceVariant<FanRenderData>(renderData.layers);

		lightPolygon.points.reserve(polygon.size() + 2);
		lightPolygon.points.push_back(ZERO_VECTOR);
		for (auto& point : polygon)
		{
			lightPolygon.points.push_back(point);
		}
		lightPolygon.points.push_back(polygon[0]);

		lightPolygon.spriteHandle = lightSpriteHandle;
		lightPolygon.alpha = 0.5f;
		lightPolygon.start = drawShift - fovSize;
		lightPolygon.size = fovSize;
	}
}

void RenderSystem::drawBackground(RenderData& renderData, World& world, Vector2D drawShift, Vector2D windowSize)
{
	SCOPED_PROFILER("RenderSystem::drawBackground");
	auto [backgroundTexture] = world.getWorldComponents().getComponents<BackgroundTextureComponent>();
	if (backgroundTexture != nullptr)
	{
		if (!backgroundTexture->getSprite().spriteHandle.isValid())
		{
			backgroundTexture->getSpriteRef().spriteHandle = mResourceManager.lockResource<Graphics::Sprite>(backgroundTexture->getSpriteDesc().path);
			backgroundTexture->getSpriteRef().params = backgroundTexture->getSpriteDesc().params;
		}

		const SpriteData& spriteData = backgroundTexture->getSpriteRef();
		const Vector2D spriteSize(spriteData.params.size);
		const Vector2D tiles(windowSize.x / spriteSize.x, windowSize.y / spriteSize.y);
		const Vector2D uvShift(-drawShift.x / spriteSize.x, -drawShift.y / spriteSize.y);

		BackgroundRenderData& bgRenderData = TemplateHelpers::EmplaceVariant<BackgroundRenderData>(renderData.layers);
		bgRenderData.spriteHandle = spriteData.spriteHandle;
		bgRenderData.start = ZERO_VECTOR;
		bgRenderData.size = windowSize;
		bgRenderData.uv = Graphics::QuadUV(uvShift, uvShift + tiles);
	}
}

struct VisibilityPolygonCalculationResult
{
	std::vector<Vector2D> polygon;
	Vector2D location;
	Vector2D size;
};

using LightBlockingComponents = std::vector<const LightBlockingGeometryComponent*>;

static std::vector<VisibilityPolygonCalculationResult> processVisibilityPolygonsGroup(
	const TupleVector<LightComponent*, const TransformComponent*>& componentsToProcess,
	Vector2D maxFov,
	const LightBlockingComponents& lightBlockingComponents,
	const GameplayTimestamp& timestamp)
{
	SCOPED_PROFILER("processVisibilityPolygonsGroup()");

	std::vector<VisibilityPolygonCalculationResult> calculationResults(componentsToProcess.size());
	VisibilityPolygonCalculator visibilityPolygonCalculator;
	for (size_t i = 0; i < componentsToProcess.size(); ++i)
	{
		auto [light, transform] = componentsToProcess[i];

		visibilityPolygonCalculator.calculateVisibilityPolygon(light->getCachedVisibilityPolygonRef(), lightBlockingComponents, transform->getLocation(), maxFov * light->getBrightness());
		light->setUpdateTimestamp(timestamp);

		const std::vector<Vector2D>& visibilityPolygon = light->getCachedVisibilityPolygon();

		calculationResults[i].polygon.resize(visibilityPolygon.size());
		std::ranges::copy(visibilityPolygon, std::begin(calculationResults[i].polygon));
		calculationResults[i].location = transform->getLocation();
		calculationResults[i].size = maxFov * light->getBrightness();
	}
	return calculationResults;
}

static size_t GetJobDivisor(size_t maxThreadsCount)
{
	// this algorithm is subject to change
	// we need to divide work into chunks to pass to different threads
	// take to consideration that the count of free threads most likely
	// smaller that threadsCount and can fluctuate over time
	return maxThreadsCount * 3 - 1;
}

void RenderSystem::drawLights(RenderData& renderData, SpatialEntityManager& managerGroup, SpatialEntityManager::RecordsVector& cells, Vector2D playerSightPosition, Vector2D drawShift, Vector2D maxFov, Vector2D screenHalfSize, const GameplayTimestamp& timestampNow)
{
	SCOPED_PROFILER("RenderSystem::drawLights");

	// get all the collidable components
	std::vector<const LightBlockingGeometryComponent*> lightBlockingComponents;
	lightBlockingComponents.reserve(cells.size());
	for (SpatialEntityManager::Record& record : cells)
	{
		auto [lightBlockingGeometry] = record.extraData.get().getCellComponents().getComponents<const LightBlockingGeometryComponent>();
		if (lightBlockingGeometry) [[likely]]
		{
			lightBlockingComponents.push_back(lightBlockingGeometry);
		}
	}

	// get lights
	TupleVector<LightComponent*, const TransformComponent*> lightComponentSets;
	managerGroup.getComponents<LightComponent, const TransformComponent>(lightComponentSets);

	// determine the borders of the location we're interested in
	Vector2D emitterPositionBordersLT = playerSightPosition - screenHalfSize - maxFov;
	Vector2D emitterPositionBordersRB = playerSightPosition + screenHalfSize + maxFov;

	// exclude lights that are too far to be visible
	std::erase_if(
		lightComponentSets,
		[emitterPositionBordersLT, emitterPositionBordersRB](auto& componentSet)
		{
			return !std::get<1>(componentSet)->getLocation().isInsideRect(emitterPositionBordersLT, emitterPositionBordersRB);
		}
	);

	if (!lightComponentSets.empty())
	{
		// collect the results from all the threads to one vector
		std::vector<VisibilityPolygonCalculationResult> allResults;
		allResults.reserve(lightComponentSets.size());

		// prepare function that will collect the calculated data
		auto finalizeFn = [&allResults](std::any&& result)
		{
			SCOPED_PROFILER("finalizeFn()");
			std::ranges::move(
				std::any_cast<std::vector<VisibilityPolygonCalculationResult>&>(result),
				std::back_inserter(allResults)
			);
		};

		// calculate how many threads we need
		const size_t threadsCount = mThreadPool.getThreadsCount();
		const size_t minimumComponentsForThread = 3;
		const size_t componentsToRecalculate = lightComponentSets.size();
		const size_t chunksCount = std::max(static_cast<size_t>(1), std::min(GetJobDivisor(threadsCount + 1), componentsToRecalculate / minimumComponentsForThread));
		const size_t chunkSize = componentsToRecalculate / chunksCount;

		std::vector<std::pair<std::function<std::any()>, std::function<void(std::any&&)>>> jobs;
		jobs.reserve(chunksCount);

		// fill the jobs
		TupleVector<LightComponent*, const TransformComponent*> oneTaskComponents;
		oneTaskComponents.reserve(chunkSize);
		for (size_t i = 0; i < chunksCount; ++i)
		{
			std::move(
				lightComponentSets.begin() + (i * chunkSize),
				lightComponentSets.begin() + (std::min(lightComponentSets.size(), (i + 1) * chunkSize)),
				std::back_inserter(oneTaskComponents)
			);

			jobs.emplace_back(
				[components = std::move(oneTaskComponents), maxFov, &lightBlockingComponents, timestampNow]()
				{
					return processVisibilityPolygonsGroup(components, maxFov, lightBlockingComponents, timestampNow);
				},
				finalizeFn
			);
			oneTaskComponents.clear();
		}

		// start heavy calculations
		mThreadPool.executeTasks(std::move(jobs), 1u);
		mThreadPool.processAndFinalizeTasks(1u);

		// sort lights in some deterministic order
		std::ranges::sort(allResults, [](auto& a, auto& b)
		{
			return (
				a.location.x < b.location.x
				||
				(a.location.x == b.location.x && a.location.y < b.location.y)
			);
		});

		// draw the results on screen
		for (auto& result : allResults)
		{
			DrawVisibilityPolygon(
				renderData,
				mLightSpriteHandle,
				result.polygon,
				result.size,
				drawShift + result.location
			);
		}
	}

	// draw player visibility polygon
	VisibilityPolygonCalculator visibilityPolygonCalculator;
	std::vector<Vector2D> polygon;
	visibilityPolygonCalculator.calculateVisibilityPolygon(polygon, lightBlockingComponents, playerSightPosition, maxFov);
	DrawVisibilityPolygon(renderData, mLightSpriteHandle, polygon, maxFov, drawShift + playerSightPosition);
}

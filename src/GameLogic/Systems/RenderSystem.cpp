#include "Base/precomp.h"

#include "GameLogic/Systems/RenderSystem.h"

#include <algorithm>
#include <ranges>

#include "Base/Types/TemplateAliases.h"
#include "Base/Types/TemplateHelpers.h"

#include "GameData/GameData.h"
#include "GameData/World.h"

#include "Utils/Geometry/VisibilityPolygon.h"

#include "GameLogic/Render/RenderAccessor.h"

#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

RenderSystem::RenderSystem(
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
		Jobs::WorkerManager& jobsWorkerManager
	) noexcept
	: mWorldCachedDataFilter(std::move(worldCachedDataFilter))
	, mRenderModeFilter(std::move(renderModeFilter))
	, mBackgroundTextureFilter(std::move(backgroundTextureFilter))
	, mLightBlockingGeometryFilter(std::move(lightBlockingGeometryFilter))
	, mSpriteRenderFilter(std::move(spriteRenderFilter))
	, mLightFilter(std::move(lightFilter))
	, mRenderAccessorFilter(std::move(renderAccessorFilter))
	, mWorldHolder(worldHolder)
	, mTime(timeData)
	, mResourceManager(resourceManager)
	, mJobsWorkerManager(jobsWorkerManager)
{
	mLightSpriteHandle = resourceManager.lockSprite("resources/textures/light.png");
}

void RenderSystem::update()
{
	World& world = mWorldHolder.getWorld();
	GameData& gameData = mWorldHolder.getGameData();

	const auto [worldCachedData] = mWorldCachedDataFilter.getComponents(world.getWorldComponents());
	Vector2D workingRect = worldCachedData->getScreenSize();
	Vector2D cameraLocation = worldCachedData->getCameraPos();

	static const Vector2D maxFov(500.0f, 500.0f);

	const auto [renderMode] = mRenderModeFilter.getComponents(gameData.getGameComponents());

	Vector2D halfWindowSize = workingRect * 0.5f;

	Vector2D drawShift = halfWindowSize - cameraLocation;

	std::vector<WorldCell*> cells = world.getSpatialData().getCellsAround(cameraLocation, workingRect);
	SpatialEntityManager spatialManager(cells);

	RenderAccessor* renderAccessor = nullptr;
	if (auto [renderAccessorCmp] = gameData.getGameComponents().getComponents<RenderAccessorComponent>(); renderAccessorCmp != nullptr)
	{
		renderAccessor = renderAccessorCmp->getAccessor();
	}

	if (renderAccessor == nullptr)
	{
		return;
	}

	std::unique_ptr<RenderData> renderData = std::make_unique<RenderData>();

	if (!renderMode || renderMode->getIsDrawBackgroundEnabled())
	{
		drawBackground(*renderData, world, drawShift, workingRect);
	}

	if (!renderMode || renderMode->getIsDrawLightsEnabled())
	{
		drawLights(*renderData, spatialManager, cells, cameraLocation, drawShift, maxFov, halfWindowSize);
	}

	if (!renderMode || renderMode->getIsDrawVisibleEntitiesEnabled())
	{
		spatialManager.forEachComponentSet(
			mSpriteRenderFilter,
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

	renderAccessor->submitData(std::move(renderData));
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
	auto [backgroundTexture] = mBackgroundTextureFilter.getComponents(world.getWorldComponents());
	if (backgroundTexture != nullptr)
	{
		if (!backgroundTexture->getSprite().spriteHandle.isValid())
		{
			backgroundTexture->getSpriteRef().spriteHandle = mResourceManager.lockSprite(backgroundTexture->getSpriteDesc().path);
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

class VisibilityPolygonCalculationJob : public Jobs::BaseJob
{
public:
	struct Result
	{
		Result() = default;
		Result(const std::vector<Vector2D>& polygon, Vector2D location)
			: polygon(polygon)
			, location(location)
		{
		}

		std::vector<Vector2D> polygon;
		Vector2D location;
		Vector2D size;
	};

	using FinalizeFn = std::function<void(std::vector<Result>&&)>;
	using LightBlockingComponents = std::vector<const LightBlockingGeometryComponent*>;

public:
	VisibilityPolygonCalculationJob(Vector2D maxFov, const LightBlockingComponents& lightBlockingComponents, GameplayTimestamp timestamp, FinalizeFn finalizeFn) noexcept
		: mMaxFov(maxFov)
		, mLightBlockingComponents(lightBlockingComponents)
		, mTimestamp(timestamp)
		, mFinalizeFn(std::move(finalizeFn))
	{
		Assert(mFinalizeFn, "finalizeFn should be set");
	}

	~VisibilityPolygonCalculationJob() override;

	void process() override
	{
		VisibilityPolygonCalculator visibilityPolygonCalculator;
		mCalculationResults.resize(componentsToProcess.size());
		for (size_t i = 0; i < componentsToProcess.size(); ++i)
		{
			auto [light, transform] = componentsToProcess[i];

			visibilityPolygonCalculator.calculateVisibilityPolygon(light->getCachedVisibilityPolygonRef(), mLightBlockingComponents, transform->getLocation(), mMaxFov * light->getBrightness());
			light->setUpdateTimestamp(mTimestamp);

			const std::vector<Vector2D>& visibilityPolygon = light->getCachedVisibilityPolygon();

			mCalculationResults[i].polygon.resize(visibilityPolygon.size());
			std::ranges::copy(visibilityPolygon, std::begin(mCalculationResults[i].polygon));
			mCalculationResults[i].location = transform->getLocation();
			mCalculationResults[i].size = mMaxFov * light->getBrightness();
		}
	}

	void finalize() override
	{
		if (mFinalizeFn)
		{
			mFinalizeFn(std::move(mCalculationResults));
		}
	}

public:
	TupleVector<LightComponent*, const TransformComponent*> componentsToProcess;

private:
	Vector2D mMaxFov;
	const LightBlockingComponents& mLightBlockingComponents;
	const GameplayTimestamp mTimestamp;
	FinalizeFn mFinalizeFn;

	std::vector<Result> mCalculationResults;
};

// just to suppress weak vtables warning
VisibilityPolygonCalculationJob::~VisibilityPolygonCalculationJob() = default;

static size_t GetJobDivisor(size_t maxThreadsCount)
{
	// this algorithm is subject to change
	// we need to divide work into chunks to pass to different threads
	// take to consideration that the count of free threads most likely
	// smaller that threadsCount and can fluctuate over time
	return maxThreadsCount * 3 - 1;
}

void RenderSystem::drawLights(RenderData& renderData, SpatialEntityManager& managerGroup, std::vector<WorldCell*>& cells, Vector2D playerSightPosition, Vector2D drawShift, Vector2D maxFov, Vector2D screenHalfSize)
{
	const GameplayTimestamp timestampNow = mTime.currentTimestamp;

	// get all the collidable components
	std::vector<const LightBlockingGeometryComponent*> lightBlockingComponents;
	lightBlockingComponents.reserve(cells.size());
	for (WorldCell* cell : cells)
	{
		auto [lightBlockingGeometry] = mLightBlockingGeometryFilter.getComponents(cell->getCellComponents());
		if ALMOST_ALWAYS(lightBlockingGeometry)
		{
			lightBlockingComponents.push_back(lightBlockingGeometry);
		}
	}

	// get lights
	TupleVector<LightComponent*, const TransformComponent*> lightComponentSets;
	managerGroup.getComponents(mLightFilter, lightComponentSets);

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
		std::vector<VisibilityPolygonCalculationJob::Result> allResults;
		allResults.reserve(lightComponentSets.size());

		// prepare function that will collect the calculated data
		auto finalizeFn = [&allResults](std::vector<VisibilityPolygonCalculationJob::Result>&& results)
		{
			std::ranges::move(results, std::back_inserter(allResults));
		};

		// calculate how many threads we need
		size_t threadsCount = mJobsWorkerManager.getThreadsCount();
		AssertFatal(threadsCount != 0, "Jobs Worker Manager threads count can't be zero");
		size_t chunksCount = GetJobDivisor(threadsCount + 1);
		size_t componentsToRecalculate = lightComponentSets.size();
		size_t chunkSize = std::max((componentsToRecalculate / chunksCount) + (componentsToRecalculate % chunksCount > 1 ? 1 : 0), static_cast<size_t>(1));

		std::vector<Jobs::BaseJob::UniquePtr> jobs;
		size_t chunkItemIndex = 0;

		// fill the jobs
		for (const auto& lightData : lightComponentSets)
		{
			if (chunkItemIndex == 0)
			{
				jobs.emplace_back(std::make_unique<VisibilityPolygonCalculationJob>(maxFov, lightBlockingComponents, timestampNow, finalizeFn));
			}

			VisibilityPolygonCalculationJob* jobData = static_cast<VisibilityPolygonCalculationJob*>(jobs.rbegin()->get());

			jobData->componentsToProcess.emplace_back(lightData);

			++chunkItemIndex;
			if (chunkItemIndex >= chunkSize)
			{
				chunkItemIndex = 0;
			}
		}

		// start heavy calculations
		mJobsWorkerManager.runJobs(std::move(jobs));

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

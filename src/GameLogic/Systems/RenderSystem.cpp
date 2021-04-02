#include "Base/precomp.h"

#include "GameLogic/Systems/RenderSystem.h"

#include <algorithm>
#include <ranges>

#include "GameData/Components/RenderComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/LightComponent.generated.h"
#include "GameData/Components/LightBlockingGeometryComponent.generated.h"
#include "GameData/Components/RenderModeComponent.generated.h"
#include "GameData/Components/WorldCachedDataComponent.generated.h"
#include "GameData/Components/BackgroundTextureComponent.generated.h"
#include "GameData/GameData.h"
#include "GameData/World.h"

#include "Utils/Geometry/VisibilityPolygon.h"

#include "HAL/Base/Math.h"
#include "HAL/Base/Engine.h"
#include "HAL/Graphics/Sprite.h"

#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

RenderSystem::RenderSystem(
		WorldHolder& worldHolder,
		const TimeData& timeData,
		HAL::Engine& engine,
		HAL::ResourceManager& resourceManager,
		Jobs::WorkerManager& jobsWorkerManager
	)
	: mWorldHolder(worldHolder)
	, mTime(timeData)
	, mEngine(engine)
	, mResourceManager(resourceManager)
	, mJobsWorkerManager(jobsWorkerManager)
{
	mLightSpriteHandle = resourceManager.lockSprite("resources/textures/light.png");
}

void RenderSystem::update()
{
	World& world = mWorldHolder.getWorld();
	GameData& gameData = mWorldHolder.getGameData();

	auto [worldCachedData] = world.getWorldComponents().getComponents<WorldCachedDataComponent>();
	Vector2D workingRect = worldCachedData->getScreenSize();
	Vector2D cameraLocation = worldCachedData->getCameraPos();

	static const Vector2D maxFov(500.0f, 500.0f);

	auto [renderMode] = gameData.getGameComponents().getComponents<RenderModeComponent>();

	Vector2D screenHalfSize = mEngine.getWindowSize() * 0.5f;

	Vector2D drawShift = screenHalfSize - cameraLocation;

	std::vector<WorldCell*> cells = world.getSpatialData().getCellsAround(cameraLocation, workingRect);
	SpatialEntityManager spatialManager(cells);

	if (!renderMode || renderMode->getIsDrawBackgroundEnabled())
	{
		drawBackground(world, drawShift);
	}

	if (!renderMode || renderMode->getIsDrawLightsEnabled())
	{
		drawLights(spatialManager, cells, cameraLocation, drawShift, maxFov, screenHalfSize);
	}

	if (!renderMode || renderMode->getIsDrawVisibleEntitiesEnabled())
	{
		spatialManager.forEachComponentSet<RenderComponent, TransformComponent>([&drawShift, &resourceManager = mResourceManager](RenderComponent* render, TransformComponent* transform)
		{
			Vector2D location = transform->getLocation() + drawShift;
			float rotation = transform->getRotation().getValue();
			for (const auto& data : render->getSpriteDatas())
			{
				const Graphics::Sprite& spriteData = resourceManager.getResource<Graphics::Sprite>(data.spriteHandle);
				Graphics::Render::drawQuad(*spriteData.getSurface(), location, data.params.size, data.params.anchor, rotation, spriteData.getUV(), 1.0f);
			}
		});
	}
}

void RenderSystem::drawVisibilityPolygon(const Graphics::Sprite& lightSprite, const std::vector<Vector2D>& polygon, const Vector2D& fowSize, const Vector2D& drawShift)
{
	if (polygon.size() > 2)
	{
		Graphics::QuadUV quadUV = lightSprite.getUV();

		std::vector<Graphics::DrawPoint> drawablePolygon;
		drawablePolygon.reserve(polygon.size() + 2);
		drawablePolygon.push_back(Graphics::DrawPoint{ZERO_VECTOR, Graphics::QuadLerp(quadUV, 0.5f, 0.5f)});
		for (auto& point : polygon)
		{
			drawablePolygon.push_back(Graphics::DrawPoint{point, Graphics::QuadLerp(quadUV, 0.5f-point.x/fowSize.x, 0.5f-point.y/fowSize.y)});
		}
		drawablePolygon.push_back(Graphics::DrawPoint{polygon[0], Graphics::QuadLerp(quadUV, 0.5f-polygon[0].x/fowSize.x, 0.5f-polygon[0].y/fowSize.y)});

		glm::mat4 transform(1.0f);
		transform = glm::translate(transform, glm::vec3(drawShift.x, drawShift.y, 0.0f));
		Graphics::Render::drawFan(*lightSprite.getSurface(), drawablePolygon, transform, 0.5f);
	}
}

void RenderSystem::drawBackground(World& world, const Vector2D& drawShift)
{
	auto [backgroundTexture] = world.getWorldComponents().getComponents<BackgroundTextureComponent>();
	if (backgroundTexture != nullptr)
	{
		if (!backgroundTexture->getSprite().spriteHandle.isValid())
		{
			backgroundTexture->getSpriteRef().spriteHandle = mResourceManager.lockSprite(backgroundTexture->getSpriteDesc().path);
			backgroundTexture->getSpriteRef().params = backgroundTexture->getSpriteDesc().params;
		}

		const SpriteData& spriteData = backgroundTexture->getSpriteRef();
		const Graphics::Sprite& backgroundSprite = mResourceManager.getResource<Graphics::Sprite>(spriteData.spriteHandle);
		const Vector2D windowSize = mEngine.getWindowSize();
		const Vector2D spriteSize(spriteData.params.size);
		const Vector2D tiles(windowSize.x / spriteSize.x, windowSize.y / spriteSize.y);
		const Vector2D uvShift(-drawShift.x / spriteSize.x, -drawShift.y / spriteSize.y);
		Graphics::Render::drawTiledQuad(*backgroundSprite.getSurface(), ZERO_VECTOR, windowSize, tiles, uvShift);
	}
}

Vector2D RenderSystem::GetPlayerSightPosition(World& world)
{
	Vector2D result(0.0f, 0.0f);

	std::optional<std::pair<EntityView, CellPos>> controlledEntity = world.getTrackedSpatialEntity(STR_TO_ID("ControlledEntity"));
	if (controlledEntity.has_value())
	{
		auto [playerTransform] = controlledEntity->first.getComponents<TransformComponent>();

		if (playerTransform != nullptr)
		{
			result = playerTransform->getLocation();
		}
	}

	return result;
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
	using LightBlockingComponents = std::vector<LightBlockingGeometryComponent*>;

public:
	VisibilityPolygonCalculationJob(Vector2D maxFov, const LightBlockingComponents& lightBlockingComponents, GameplayTimestamp timestamp, FinalizeFn finalizeFn)
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
	TupleVector<LightComponent*, TransformComponent*> componentsToProcess;

private:
	Vector2D mMaxFov;
	const LightBlockingComponents& mLightBlockingComponents;
	const GameplayTimestamp mTimestamp;
	FinalizeFn mFinalizeFn;

	std::vector<Result> mCalculationResults;
};

// just to suppress weak vtables warning
VisibilityPolygonCalculationJob::~VisibilityPolygonCalculationJob() {}

static size_t GetJobDivisor(size_t maxThreadsCount)
{
	// this alghorithm is subject to change
	// we need to divide work into chunks to pass to different threads
	// take to consideration that the count of free threads most likely
	// smaller that threadsCount and can fluctuate over time
	return maxThreadsCount * 3 - 1;
}

void RenderSystem::drawLights(SpatialEntityManager& managerGroup, std::vector<WorldCell*>& cells, Vector2D playerSightPosition, Vector2D drawShift, Vector2D maxFov, Vector2D screenHalfSize)
{
	const GameplayTimestamp timestampNow = mTime.currentTimestamp;

	const Graphics::Sprite& lightSprite = mResourceManager.getResource<Graphics::Sprite>(mLightSpriteHandle);
	if (!lightSprite.isValid())
	{
		return;
	}

	// get all the collidable components
	std::vector<LightBlockingGeometryComponent*> lightBlockingComponents;
	lightBlockingComponents.reserve(cells.size());
	for (WorldCell* cell : cells)
	{
		auto [lightBlockingGeometry] = cell->getCellComponents().getComponents<LightBlockingGeometryComponent>();
		if ALMOST_ALWAYS(lightBlockingGeometry)
		{
			lightBlockingComponents.push_back(lightBlockingGeometry);
		}
	}

	// get lights
	TupleVector<LightComponent*, TransformComponent*> lightComponentSets;
	managerGroup.getComponents<LightComponent, TransformComponent>(lightComponentSets);

	// determine the borders of the location we're interested in
	Vector2D emitterPositionBordersLT = playerSightPosition - screenHalfSize - maxFov;
	Vector2D emitterPositionBordersRB = playerSightPosition + screenHalfSize + maxFov;

	// exclude lights that are too far to be visible
	lightComponentSets.erase(
		std::remove_if(
			std::begin(lightComponentSets),
			std::end(lightComponentSets),
			[emitterPositionBordersLT, emitterPositionBordersRB](auto& componentSet)
			{
				return !std::get<1>(componentSet)->getLocation().isInsideRect(emitterPositionBordersLT, emitterPositionBordersRB);
			}
		),
		std::end(lightComponentSets)
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
				jobs.emplace_back(HS_NEW VisibilityPolygonCalculationJob(maxFov, lightBlockingComponents, timestampNow, finalizeFn));
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
			drawVisibilityPolygon(
				lightSprite,
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
	drawVisibilityPolygon(lightSprite, polygon, maxFov, drawShift + playerSightPosition);
}

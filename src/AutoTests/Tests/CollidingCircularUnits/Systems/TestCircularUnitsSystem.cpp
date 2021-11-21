#include "Base/precomp.h"

#include "AutoTests/Tests/CollidingCircularUnits/Systems/TestCircularUnitsSystem.h"

#include "GameData/World.h"


TestCircularUnitsSystem::TestCircularUnitsSystem(
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>&& trackedFilter,
		RaccoonEcs::ComponentFilter<const TransformComponent>&& transformFilter,
		RaccoonEcs::ComponentFilter<const AiControllerComponent, const TransformComponent, MovementComponent>&& aiMovementFilter,
		WorldHolder& worldHolder,
		const TimeData& time) noexcept
	: mTrackedFilter(std::move(trackedFilter))
	, mTransformFilter(std::move(transformFilter))
	, mAiMovementFilter(std::move(aiMovementFilter))
	, mWorldHolder(worldHolder)
	, mTime(time)
{
}

void TestCircularUnitsSystem::update()
{
	World& world = mWorldHolder.getWorld();
	float dt = mTime.dt;

	std::optional<std::pair<AsyncEntityView, CellPos>> playerEntity = world.getTrackedSpatialEntity(mTrackedFilter, STR_TO_ID("ControlledEntity"));
	if (!playerEntity.has_value())
	{
		return;
	}

	const auto [playerTransform] = playerEntity->first.getComponents(mTransformFilter);
	if (playerTransform == nullptr)
	{
		return;
	}

	Vector2D targetLocation = playerTransform->getLocation();

	SpatialEntityManager spatialManager = world.getSpatialData().getAllCellManagers();
	spatialManager.forEachComponentSet(
		mAiMovementFilter,
		[targetLocation, dt](const AiControllerComponent* /*aiController*/, const TransformComponent* transform, MovementComponent* movement)
		{
			Vector2D nextStep = targetLocation - transform->getLocation();
			movement->setMoveDirection(nextStep);
			movement->setNextStep(nextStep * movement->getOriginalSpeed() * dt);
			movement->setSpeed(movement->getOriginalSpeed());
		}
	);
}

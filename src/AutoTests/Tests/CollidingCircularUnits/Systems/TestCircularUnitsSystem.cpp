#include "AutoTests/Tests/CollidingCircularUnits/Systems/TestCircularUnitsSystem.h"

#include "GameData/World.h"

#include "GameData/Components/NavMeshComponent.generated.h"
#include "GameData/Components/AiControllerComponent.generated.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"


TestCircularUnitsSystem::TestCircularUnitsSystem(WorldHolder& worldHolder, TimeData& time)
	: mWorldHolder(worldHolder)
	, mTime(time)
{
}

void TestCircularUnitsSystem::update()
{
	World& world = mWorldHolder.getWorld();
	float dt = mTime.dt;

	std::optional<std::pair<EntityView, CellPos>> playerEntity = world.getTrackedSpatialEntity(STR_TO_ID("ControlledEntity"));
	if (!playerEntity.has_value())
	{
		return;
	}

	auto [playerTransform] = playerEntity->first.getComponents<TransformComponent>();
	if (playerTransform == nullptr)
	{
		return;
	}

	Vector2D targetLocation = playerTransform->getLocation();
	CellPos targetCell = playerEntity->second;

	SpatialEntityManager spatialManager = world.getSpatialData().getAllCellManagers();
	spatialManager.forEachSpatialComponentSet<AiControllerComponent, TransformComponent, MovementComponent>([targetLocation, targetCell, dt](WorldCell* cell, AiControllerComponent* /*aiController*/, TransformComponent* transform, MovementComponent* movement)
	{
		Vector2D cellPosDiff = SpatialWorldData::GetCellRealDistance(targetCell - cell->getPos());
		Vector2D nextStep = targetLocation - transform->getLocation() + cellPosDiff;
		movement->setMoveDirection(nextStep);
		movement->setNextStep(nextStep * movement->getOriginalSpeed() * dt);
		movement->setSpeed(movement->getOriginalSpeed());
	});
}

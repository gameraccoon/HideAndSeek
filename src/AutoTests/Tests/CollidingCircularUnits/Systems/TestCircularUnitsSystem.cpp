#include "Base/precomp.h"

#include "AutoTests/Tests/CollidingCircularUnits/Systems/TestCircularUnitsSystem.h"

#include "GameData/Components/AiControllerComponent.generated.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"
#include "GameData/Components/NavMeshComponent.generated.h"
#include "GameData/Components/TimeComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/World.h"

#include "GameLogic/SharedManagers/WorldHolder.h"


TestCircularUnitsSystem::TestCircularUnitsSystem(WorldHolder& worldHolder) noexcept
	: mWorldHolder(worldHolder)
{
}

void TestCircularUnitsSystem::update()
{
	World& world = mWorldHolder.getWorld();

	const auto [time] = world.getWorldComponents().getComponents<TimeComponent>();
	const float dt = time->getValue().lastFixedUpdateDt;

	std::optional<std::pair<EntityView, CellPos>> playerEntity = world.getTrackedSpatialEntity(STR_TO_ID("ControlledEntity"));
	if (!playerEntity.has_value())
	{
		return;
	}

	const auto [playerTransform] = playerEntity->first.getComponents<const TransformComponent>();
	if (playerTransform == nullptr)
	{
		return;
	}

	Vector2D targetLocation = playerTransform->getLocation();

	SpatialEntityManager spatialManager = world.getSpatialData().getAllCellManagers();
	spatialManager.forEachComponentSet<const AiControllerComponent, const TransformComponent, MovementComponent>(
		[targetLocation, dt](const AiControllerComponent* /*aiController*/, const TransformComponent* transform, MovementComponent* movement)
		{
			Vector2D nextStep = targetLocation - transform->getLocation();
			movement->setMoveDirection(nextStep);
			movement->setNextStep(nextStep * movement->getOriginalSpeed() * dt);
			movement->setSpeed(movement->getOriginalSpeed());
		}
	);
}

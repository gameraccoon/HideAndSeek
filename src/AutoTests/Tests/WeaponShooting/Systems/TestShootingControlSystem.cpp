#include "Base/precomp.h"

#include "AutoTests/Tests/WeaponShooting/Systems/TestShootingControlSystem.h"

#include <limits>

#include "GameData/World.h"

#include "GameData/Components/HealthComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/WeaponComponent.generated.h"
#include "GameData/Components/CharacterStateComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"


TestShootingControlSystem::TestShootingControlSystem(
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>&& trackedFilter,
		WorldHolder& worldHolder,
		TimeData& time) noexcept
	: mTrackedFilter(std::move(trackedFilter))
	, mWorldHolder(worldHolder)
	, mTime(time)
{
}

void TestShootingControlSystem::update()
{
	World& world = mWorldHolder.getWorld();

	std::optional<std::pair<EntityView, CellPos>> playerEntity = world.getTrackedSpatialEntity(mTrackedFilter, STR_TO_ID("ControlledEntity"));
	if (!playerEntity.has_value())
	{
		return;
	}

	auto [playerTransform, playerWeapon, characterState, movement] = playerEntity->first.getComponents<TransformComponent, WeaponComponent, CharacterStateComponent, MovementComponent>();
	if (playerTransform == nullptr || playerWeapon == nullptr || characterState == nullptr)
	{
		return;
	}

	Vector2D playerLocation = playerTransform->getLocation();
	Vector2D closestTarget;
	float closestQDist = std::numeric_limits<float>::max();

	SpatialEntityManager spatialManager = world.getSpatialData().getAllCellManagers();
	spatialManager.forEachComponentSet<HealthComponent, TransformComponent>([playerLocation, &closestTarget, &closestQDist](HealthComponent* /*health*/, TransformComponent* transform)
	{
		float qDist = (transform->getLocation() - playerLocation).qSize();
		if (qDist < closestQDist)
		{
			closestQDist = qDist;
			closestTarget = transform->getLocation();
		}
	});

	float weaponShootDistance = playerWeapon->getShotDistance();

	bool canShoot = closestQDist < weaponShootDistance*weaponShootDistance;
	characterState->getBlackboardRef().setValue<bool>(CharacterStateBlackboardKeys::TryingToShoot, canShoot);
	movement->setSightDirection(closestTarget - playerLocation);
}

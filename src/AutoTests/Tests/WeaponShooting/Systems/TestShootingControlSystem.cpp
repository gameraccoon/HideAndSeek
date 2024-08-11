#include "EngineCommon/precomp.h"

#include "AutoTests/Tests/WeaponShooting/Systems/TestShootingControlSystem.h"

#include <limits>

#include "GameData/Components/CharacterStateComponent.generated.h"
#include "GameData/Components/HealthComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/WeaponComponent.generated.h"
#include "GameData/World.h"

#include "GameLogic/SharedManagers/WorldHolder.h"

TestShootingControlSystem::TestShootingControlSystem(WorldHolder& worldHolder) noexcept
	: mWorldHolder(worldHolder)
{
}

void TestShootingControlSystem::update()
{
	World& world = mWorldHolder.getWorld();

	std::optional<std::pair<EntityView, CellPos>> playerEntity = world.getTrackedSpatialEntity(STR_TO_ID("ControlledEntity"));
	if (!playerEntity.has_value())
	{
		return;
	}

	auto [playerTransform, playerWeapon, characterState, movement] = playerEntity->first.getComponents<const TransformComponent, const WeaponComponent, CharacterStateComponent, MovementComponent>();
	if (playerTransform == nullptr || playerWeapon == nullptr || characterState == nullptr || movement == nullptr)
	{
		return;
	}

	Vector2D playerLocation = playerTransform->getLocation();
	Vector2D closestTarget;
	float closestQDist = std::numeric_limits<float>::max();

	world.getSpatialData().getAllCellManagers().forEachComponentSet<const HealthComponent, const TransformComponent>(
		[playerLocation, &closestTarget, &closestQDist](const HealthComponent* health, const TransformComponent* transform) {
			const float qDist = (transform->getLocation() - playerLocation).qSize();
			if (health->getHealthValue() > 0.0f && qDist < closestQDist)
			{
				closestQDist = qDist;
				closestTarget = transform->getLocation();
			}
		}
	);

	const float weaponShootDistance = playerWeapon->getShotDistance();

	const bool canShoot = closestQDist < weaponShootDistance * weaponShootDistance;
	characterState->getBlackboardRef().setValue<bool>(CharacterStateBlackboardKeys::TryingToShoot, canShoot);
	movement->setSightDirection(closestTarget - playerLocation);
}

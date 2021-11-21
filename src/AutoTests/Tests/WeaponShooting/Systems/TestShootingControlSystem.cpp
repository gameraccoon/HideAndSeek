#include "Base/precomp.h"

#include "AutoTests/Tests/WeaponShooting/Systems/TestShootingControlSystem.h"

#include <limits>

#include "GameData/World.h"


TestShootingControlSystem::TestShootingControlSystem(
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>&& trackedFilter,
		RaccoonEcs::ComponentFilter<const TransformComponent, const WeaponComponent, CharacterStateComponent, MovementComponent>&& shooterFilter,
		RaccoonEcs::ComponentFilter<const HealthComponent, const TransformComponent>&& targetsFilter,
		WorldHolder& worldHolder,
		const TimeData& time) noexcept
	: mTrackedFilter(std::move(trackedFilter))
	, mShooterFilters(std::move(shooterFilter))
	, mTargetsFilter(std::move(targetsFilter))
	, mWorldHolder(worldHolder)
	, mTime(time)
{
}

void TestShootingControlSystem::update()
{
	World& world = mWorldHolder.getWorld();

	std::optional<std::pair<AsyncEntityView, CellPos>> playerEntity = world.getTrackedSpatialEntity(mTrackedFilter, STR_TO_ID("ControlledEntity"));
	if (!playerEntity.has_value())
	{
		return;
	}

	auto [playerTransform, playerWeapon, characterState, movement] = playerEntity->first.getComponents(mShooterFilters);
	if (playerTransform == nullptr || playerWeapon == nullptr || characterState == nullptr)
	{
		return;
	}

	Vector2D playerLocation = playerTransform->getLocation();
	Vector2D closestTarget;
	float closestQDist = std::numeric_limits<float>::max();

	world.getSpatialData().getAllCellManagers().forEachComponentSet(
			mTargetsFilter,
			[playerLocation, &closestTarget, &closestQDist](const HealthComponent* health, const TransformComponent* transform)
	{
		float qDist = (transform->getLocation() - playerLocation).qSize();
		if (health->getHealthValue() > 0.0f && qDist < closestQDist)
		{
			closestQDist = qDist;
			closestTarget = transform->getLocation();
		}
	});

	float weaponShootDistance = playerWeapon->getShotDistance();

	bool canShoot = closestQDist < weaponShootDistance * weaponShootDistance;
	characterState->getBlackboardRef().setValue<bool>(CharacterStateBlackboardKeys::TryingToShoot, canShoot);
	movement->setSightDirection(closestTarget - playerLocation);
}

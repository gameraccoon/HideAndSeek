#include "Base/precomp.h"

#include "GameLogic/Systems/WeaponSystem.h"

#include "GameData/World.h"
#include "GameData/GameData.h"

#include "GameData/Components/WeaponComponent.generated.h"
#include "GameData/Components/CharacterStateComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/HealthComponent.generated.h"
#include "GameData/Components/DeathComponent.generated.h"
#include "GameData/Spatial/SpatialEntity.h"

#include "Utils/Geometry/RayTrace.h"


WeaponSystem::WeaponSystem(
		RaccoonEcs::ComponentFilter<WeaponComponent, CharacterStateComponent>&& stateAndWeaponFilter,
		RaccoonEcs::ComponentFilter<const TransformComponent>&& transformFilter,
		RaccoonEcs::ComponentFilter<HealthComponent>&& healthFilter,
		RaccoonEcs::ComponentAdder<DeathComponent>&& deathAdder,
		WorldHolder& worldHolder,
		const TimeData& timeData) noexcept
	: mStateAndWeaponFilter(std::move(stateAndWeaponFilter))
	, mTransformFilter(std::move(transformFilter))
	, mHealthFilter(std::move(healthFilter))
	, mDeathAdder(std::move(deathAdder))
	, mWorldHolder(worldHolder)
	, mTime(timeData)
{
}

struct ShotInfo
{
	ShotInfo(Entity instigator) : instigator(instigator) {}

	Entity instigator;
	WorldCell* instigatorCell;
	float distance;
	float damage;
};

struct HitInfo
{
	explicit HitInfo(Entity instigator) : instigator(instigator) {}

	Entity instigator;
	WorldCell* instigatorCell;
	SpatialEntity hitEntity;
	Vector2D impulse;
	float damageValue;
};

void WeaponSystem::update()
{
	World& world = mWorldHolder.getWorld();
	GameplayTimestamp currentTime = mTime.currentTimestamp;

	std::vector<ShotInfo> shotsToMake;
	world.getSpatialData().getAllCellManagers().forEachSpatialComponentSetWithEntityN(
				mStateAndWeaponFilter,
				[currentTime, &shotsToMake](WorldCell* cell, Entity entity, WeaponComponent* weapon, CharacterStateComponent* characterState)
	{
		if (characterState->getState() == CharacterState::Shoot || characterState->getState() == CharacterState::WalkAndShoot)
		{
			if (currentTime > weapon->getShotFinishTimestamp())
			{
				ShotInfo shot(entity);
				shot.instigatorCell = cell;
				shot.distance = weapon->getShotDistance();
				shot.damage = weapon->getDamageValue();
				shotsToMake.push_back(shot);

				weapon->setShotFinishTimestamp(currentTime.getIncreasedByFloatTime(weapon->getShotPeriod()));
			}
		}
	});

	std::vector<HitInfo> hitsDone;
	for (const ShotInfo& shotInfo : shotsToMake)
	{
		auto [transform] = mTransformFilter.getEntityComponents(shotInfo.instigatorCell->getEntityManager(), shotInfo.instigator);
		if (transform)
		{
			Vector2D traceEndPoint = transform->getLocation() + Vector2D(transform->getRotation()) * shotInfo.distance;
			RayTrace::TraceResult result = RayTrace::Trace(
				world,
				transform->getLocation(),
				traceEndPoint
			);

			if (result.hasHit)
			{
				HitInfo hitInfo(shotInfo.instigator);
				hitInfo.instigatorCell = shotInfo.instigatorCell;
				hitInfo.hitEntity = result.hitEntity;
				hitInfo.impulse = traceEndPoint - transform->getLocation();
				hitInfo.damageValue = shotInfo.damage;
				hitsDone.push_back(hitInfo);
			}
		}
	}

	for (const HitInfo& hit : hitsDone)
	{
		WorldCell* cell = world.getSpatialData().getCell(hit.hitEntity.cell);
		AssertFatal(cell != nullptr, "Cell of the hit object is not found");

		auto [health] = mHealthFilter.getEntityComponents(cell->getEntityManager(), hit.hitEntity.entity.getEntity());
		if (health == nullptr)
		{
			// entity doesn't have health
			continue;
		}

		float healthValue = health->getHealthValue() - hit.damageValue;
		health->setHealthValue(healthValue);
		if (healthValue < 0.0f)
		{
			mDeathAdder.addComponent(cell->getEntityManager(), hit.hitEntity.entity.getEntity());
		}
	}
}

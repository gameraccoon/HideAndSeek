#include "EngineCommon/precomp.h"

#include "GameLogic/Systems/WeaponSystem.h"

#include "EngineCommon/TimeConstants.h"

#include "GameData/Components/CharacterStateComponent.generated.h"
#include "GameData/Components/DeathComponent.generated.h"
#include "GameData/Components/HealthComponent.generated.h"
#include "GameData/Components/TimeComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/WeaponComponent.generated.h"
#include "GameData/GameData.h"
#include "GameData/Spatial/SpatialEntity.h"
#include "GameData/World.h"

#include "GameUtils/Geometry/RayTrace.h"

#include "GameLogic/SharedManagers/WorldHolder.h"


WeaponSystem::WeaponSystem(WorldHolder& worldHolder) noexcept
	: mWorldHolder(worldHolder)
{
}

struct ShotInfo
{
	explicit ShotInfo(const Entity instigator) : instigator(instigator) {}

	Entity instigator;
	WorldCell* instigatorCell = nullptr;
	float distance = 0.0f;
	float damage = 0.0f;
};

struct HitInfo
{
	explicit HitInfo(const Entity instigator) : instigator(instigator) {}

	Entity instigator;
	WorldCell* instigatorCell = nullptr;
	SpatialEntity hitEntity;
	Vector2D impulse = {};
	float damageValue = 0.0f;
};

void WeaponSystem::update()
{
	SCOPED_PROFILER("WeaponSystem::update");
	World& world = mWorldHolder.getWorld();

	const auto [time] = world.getWorldComponents().getComponents<TimeComponent>();
	GameplayTimestamp currentTime = time->getValue().lastFixedUpdateTimestamp;

	std::vector<ShotInfo> shotsToMake;
	world.getSpatialData().getAllCellManagers().forEachComponentSetWithEntityAndExtraData<WeaponComponent, const CharacterStateComponent>(
		[currentTime, &shotsToMake](WorldCell& cell, const EntityView entityView, WeaponComponent* weapon, const CharacterStateComponent* characterState)
	{
		if (characterState->getState() == CharacterState::Shoot || characterState->getState() == CharacterState::WalkAndShoot)
		{
			if (currentTime > weapon->getShotFinishTimestamp())
			{
				ShotInfo shot(entityView.getEntity());
				shot.instigatorCell = &cell;
				shot.distance = weapon->getShotDistance();
				shot.damage = weapon->getDamageValue();
				shotsToMake.push_back(shot);

				weapon->setShotFinishTimestamp(currentTime.getIncreasedByUpdateCount(static_cast<s32>(weapon->getShotPeriod() / TimeConstants::ONE_FIXED_UPDATE_SEC)));
			}
		}
	});

	std::vector<HitInfo> hitsDone;
	for (const ShotInfo& shotInfo : shotsToMake)
	{
		auto [transform] = shotInfo.instigatorCell->getEntityManager().getEntityComponents<TransformComponent>(shotInfo.instigator);
		if (transform)
		{
			Vector2D traceEndPoint = transform->getLocation() + Vector2D(transform->getRotation()) * shotInfo.distance;
			const RayTrace::TraceResult result = RayTrace::Trace(
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

		auto [health] = cell->getEntityManager().getEntityComponents<HealthComponent>(hit.hitEntity.entity.getEntity());
		if (health == nullptr)
		{
			// entity doesn't have health
			continue;
		}

		float healthValue = health->getHealthValue() - hit.damageValue;
		health->setHealthValue(healthValue);
		if (healthValue < 0.0f)
		{
			cell->getEntityManager().addComponent<DeathComponent>(hit.hitEntity.entity.getEntity());
		}
	}
}

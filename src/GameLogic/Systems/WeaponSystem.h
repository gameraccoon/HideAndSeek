#pragma once

#include <unordered_map>

#include <raccoon-ecs/system.h>
#include <raccoon-ecs/async_operations.h>

#include "GameData/Components/WeaponComponent.generated.h"
#include "GameData/Components/CharacterStateComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/HealthComponent.generated.h"
#include "GameData/Components/DeathComponent.generated.h"

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

/**
 * System that handles weapon manipulation
 */
class WeaponSystem : public RaccoonEcs::System
{
public:
	WeaponSystem(
		RaccoonEcs::ComponentFilter<WeaponComponent, CharacterStateComponent>&& stateAndWeaponFilter,
		RaccoonEcs::ComponentFilter<const TransformComponent>&& transformFilter,
		RaccoonEcs::ComponentFilter<HealthComponent>&& healthFilter,
		RaccoonEcs::ComponentAdder<DeathComponent>&& deathAdder,
		RaccoonEcs::ComponentFilter<const CollisionComponent, const TransformComponent>&& collisionFilter,
		WorldHolder& worldHolder,
		const TimeData& timeData
	) noexcept;

	void update() override;
	std::string getName() const override { return "WeaponSystem"; }

private:
	RaccoonEcs::ComponentFilter<WeaponComponent, CharacterStateComponent> mStateAndWeaponFilter;
	RaccoonEcs::ComponentFilter<const TransformComponent> mTransformFilter;
	RaccoonEcs::ComponentFilter<HealthComponent> mHealthFilter;
	RaccoonEcs::ComponentAdder<DeathComponent> mDeathAdder;
	RaccoonEcs::ComponentFilter<const CollisionComponent, const TransformComponent> mCollisionFilter;
	WorldHolder& mWorldHolder;
	const TimeData& mTime;
};

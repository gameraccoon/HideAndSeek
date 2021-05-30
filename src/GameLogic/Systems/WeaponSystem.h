#pragma once

#include <unordered_map>

#include <raccoon-ecs/system.h>
#include <raccoon-ecs/async_operations.h>

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

class WeaponComponent;
class CharacterStateComponent;
class TransformComponent;
class HealthComponent;
class DeathComponent;

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
	WorldHolder& mWorldHolder;
	const TimeData& mTime;
};

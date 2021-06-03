#pragma once

#include <memory>
#include <raccoon-ecs/system.h>
#include <raccoon-ecs/async_operations.h>

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

class TrackedSpatialEntitiesComponent;
class TransformComponent;
class WeaponComponent;
class CharacterStateComponent;
class MovementComponent;
class HealthComponent;

class TestShootingControlSystem : public RaccoonEcs::System
{
public:
	TestShootingControlSystem(
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>&& trackedFilter,
		RaccoonEcs::ComponentFilter<const TransformComponent, const WeaponComponent, CharacterStateComponent, MovementComponent>&& shooterFilter,
		RaccoonEcs::ComponentFilter<const HealthComponent, const TransformComponent>&& targetsFilter,
		WorldHolder& worldHolder,
		TimeData& time
	) noexcept;

	~TestShootingControlSystem() override = default;

	void update() override;
	std::string getName() const override { return "TestShootingControlSystem"; }

private:
	RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent> mTrackedFilter;
	RaccoonEcs::ComponentFilter<const TransformComponent, const WeaponComponent, CharacterStateComponent, MovementComponent> mShooterFilters;
	RaccoonEcs::ComponentFilter<const HealthComponent, const TransformComponent> mTargetsFilter;
	WorldHolder& mWorldHolder;
	TimeData& mTime;
};

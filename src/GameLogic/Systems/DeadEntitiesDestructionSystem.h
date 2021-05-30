#pragma once

#include <unordered_map>

#include <raccoon-ecs/system.h>
#include <raccoon-ecs/async_operations.h>

#include "GameLogic/SharedManagers/WorldHolder.h"

class DeathComponent;

/**
 * System that destructs enemies marked with DeathComponent
 */
class DeadEntitiesDestructionSystem : public RaccoonEcs::System
{
public:
	explicit DeadEntitiesDestructionSystem(
		RaccoonEcs::ComponentFilter<const DeathComponent>&& deathFilter,
		RaccoonEcs::EntityRemover&& entityRemover,
		WorldHolder& worldHolder) noexcept;
	~DeadEntitiesDestructionSystem() override = default;

	void update() override;
	std::string getName() const override { return "DeadEnemiesDestructionSystem"; }

private:
	RaccoonEcs::ComponentFilter<const DeathComponent> mDeathFilter;
	RaccoonEcs::EntityRemover mEntityRemover;
	WorldHolder& mWorldHolder;
};

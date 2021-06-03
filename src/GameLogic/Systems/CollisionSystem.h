#pragma once

#include <unordered_map>

#include <raccoon-ecs/system.h>
#include <raccoon-ecs/async_operations.h>

#include "GameLogic/SharedManagers/WorldHolder.h"

class CollisionComponent;
class TransformComponent;
class MovementComponent;

/**
 * System that resolve object collisions
 */
class CollisionSystem : public RaccoonEcs::System
{
public:
	explicit CollisionSystem(
		RaccoonEcs::ComponentFilter<CollisionComponent, const TransformComponent>&& collidingFilter,
		RaccoonEcs::ComponentFilter<MovementComponent>&& movementFilter,
		RaccoonEcs::ComponentFilter<const CollisionComponent, const TransformComponent, MovementComponent>&& movingCollisionsFilter,
		WorldHolder& worldHolder) noexcept;
	~CollisionSystem() override = default;

	void update() override;
	std::string getName() const override { return "CollisionSystem"; }

private:
	RaccoonEcs::ComponentFilter<CollisionComponent, const TransformComponent> mCollidingFilter;
	RaccoonEcs::ComponentFilter<MovementComponent> mMovementFilter;
	RaccoonEcs::ComponentFilter<const CollisionComponent, const TransformComponent, MovementComponent> mMovingCollisionsFilter;
	WorldHolder& mWorldHolder;
};

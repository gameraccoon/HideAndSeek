#pragma once

#include <string>

#include "Core/Vector2D.h"
#include "Core/Rotator.h"
#include "Structures/BoundingBox.h"
#include "Structures/Hull.h"
#include "Core/ActorComponent.h"

/** Types of an actors */
enum class ActorType
{
	Ghost,		// haven't collision, may not be shown
	Static,		// static objects like wall			// added in collisions list
	Dynamic,	// movable objects like barrel		// added in collisions list and check collisions only on moving
	Living,		// player or NPC					// added in collisions list and check collisions always
	Bullet,		// basic bullets					// check collisions all life cycle but not be added in collisions list
	Special,	// doors, rockets, turrels, etc.
	Light
};

/**
 * Interface for class Actor
 */
class IActor
{
public:
	using Ptr = std::unique_ptr<IActor>;

public:
	virtual ~IActor() = default;
	/** Set new location of the actor in the World */
	virtual void setLocation(const Vector2D& location) = 0;
	/** Get actor's world location */
	virtual Vector2D getLocation() const = 0;
	/** Set actor's rotation */
	virtual void setRotation(const Rotator& rotation) = 0;
	/** Get actor's rotation */
	virtual Rotator getRotation() const = 0;
	/** Set actor's scale */
	virtual void setScale(const Vector2D& scale) = 0;
	/** Get actor's scale */
	virtual Vector2D getScale() const = 0;
	/** Process moving and other actions of the Actor */
	virtual void update(float deltatime) = 0;
	/** Say to actor, that it mast be destroyed now */
	virtual void destroy() = 0;
	/** Is actor wait to be automatically destroyed? */
	virtual bool isWaitDestruction() const = 0;
	/** Get actor type */
	virtual ActorType getType() const = 0;
	/** Get axis-aligned bounding box */
	virtual BoundingBox getBoundingBox() const = 0;
	/** Get hull of this actor */
	virtual const Hull* getGeometry() const = 0;
	/** Take some damage to the actor */
	virtual void hit(IActor *instigator, float damageValue, Vector2D impulse) = 0;
	/** Returns the class identificator of this actor's class */
	virtual std::string getClassID() const = 0;
	/** Returns the specific identificator of current object */
	virtual std::string getActorId() const = 0;

	/** Add a new component to this actor */
	virtual void AddComponent(const ActorComponent::Ptr& component) = 0;
};

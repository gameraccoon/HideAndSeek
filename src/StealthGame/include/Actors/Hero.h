#pragma once

#include <Core/World.h>
#include <Core/Vector2D.h>
#include <Actors/Body.h>

class Hero:public Body
{
public:
	/** Initialization of a new Hero standing at a given point */
	Hero(World *world, Vector2D location, Vector2D scale, Rotator rotation);

	~Hero() = default;

	/** Process moving and other actions of the man */
	virtual void update(float deltatime) override;
	
	/** Say that we want to move the man on this step */
	virtual void move(Vector2D step);

protected:
	/** Delta between needless position and current position */
	Vector2D mStep;
};

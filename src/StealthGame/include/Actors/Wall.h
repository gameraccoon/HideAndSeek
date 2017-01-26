#pragma once

#include <Core/Actor.h>

class Wall : public Actor
{
public:
	Wall(World *world, Vector2D location, Vector2D scale, Rotator rotation);
	~Wall();
	/** Dummy (wall do nothing) */
	virtual void update(float deltatime) override;
};

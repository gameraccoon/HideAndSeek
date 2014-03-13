#ifndef WALL_H
#define WALL_H

#include "../src/Actor.h"

class Wall : public Actor
{
public:
	Wall(World *ownerWorld, Vector2D location, Vector2D size);
	~Wall(void);
	/** Dummy (wall do nothing) */
	void update(float deltatime);
	/** Try to take some damage to the wall =) */
	virtual void takeDamage(float damageValue, Vector2D impulse);
private:
	/** */
	void updateCollision();
};

#endif

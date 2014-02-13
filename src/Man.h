#ifndef MAN_H
#define MAN_H

#include "../src/Globals.h"
#include "../src/Vector2D.h"
#include "../src/World.h"
#include "../src/Actor.h"
#include "../src/InventoryItem.h"
#include "../src/Weapon.h"
#include "../src/PathFinder.h"
#include "../src/DummyMan.h"

class Weapon;

class Man:public DummyMan
{
public:
	/** Initialization of a new man standing at a given point */
	Man(World *ownerWorld, Vector2D location);

	~Man();

	/** Process moving and other actions of the man */
	virtual void Update(float deltatime);

	/** Try to take some damage to the man =) */
	virtual void TakeDamage(float damageValue,Vector2D impulse);
protected:
	/**  */
	PathFinder Navigator;
	/** Point to which this man aspire */
	Vector2D DestinationPoint;
};

#endif
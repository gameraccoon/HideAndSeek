#include "Actors/Bullet.h"
#include <Components/TransformComponent.h>


Bullet::Bullet(World *world, Vector2D location, Vector2D /*scale*/, Rotator rotation)
	: Actor(world)
{
	addComponent(std::make_shared<TransformComponent>(location, rotation));
	mSpeed = 10.0f;
	setType(ActorType::Bullet);

	mSpeed = 450.0f;

	updateActorId("Bullet");
}

Bullet::~Bullet()
{
}

void Bullet::update(float deltatime)
{
	auto transformComponent = getSingleComponent<TransformComponent>();
	if (transformComponent != nullptr)
	{
		auto bulletLocation = transformComponent->getLocation();
		auto bulletRotation = transformComponent->getRotation();

		Vector2D newLocation = bulletLocation + deltatime * mSpeed * Vector2D(bulletRotation);

		Vector2D traceLocation(ZERO_VECTOR);
		IActor *trasedActor = RayTrace::trace(getOwnerWorld(), bulletLocation, newLocation, &traceLocation);

		// if there nothing to hit
		if (trasedActor == nullptr)
		{
			transformComponent->setLocation(newLocation);
		}
		else // bullet is hiting some actor
		{
			trasedActor->hit(this, 10.f, Vector2D(bulletRotation) * mSpeed * 0.01f);
			mSpeed = 0.0f;
			destroy();
		}

		// bullet will be destroyed after some time
		if (getLifetime() > 5.0f)
		{
			destroy();
		}
	}

	Actor::update(deltatime);
}

void Bullet::updateActorId(std::string classId)
{
	setClassId(classId);
	setActorId("some" + classId);
}

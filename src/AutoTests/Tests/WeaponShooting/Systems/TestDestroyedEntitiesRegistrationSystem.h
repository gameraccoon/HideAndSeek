#pragma once

#include <raccoon-ecs/utils/system.h>

class WorldHolder;
class DestroyedEntitiesTestCheck;

class TestDestroyedEntitiesRegistrationSystem : public RaccoonEcs::System
{
public:
	TestDestroyedEntitiesRegistrationSystem(
		WorldHolder& worldHolder,
		DestroyedEntitiesTestCheck& testCheck) noexcept;

	void update() override;

private:
	WorldHolder& mWorldHolder;
	DestroyedEntitiesTestCheck& mTestCheck;
};

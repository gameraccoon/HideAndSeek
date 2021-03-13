#include "Base/precomp.h"

#include "AutoTests/Tests/WeaponShooting/Systems/TestDestroyedEntitiesRegistrationSystem.h"

#include "Base/Random/Random.h"

#include "GameData/Components/DeathComponent.generated.h"
#include "GameData/World.h"

TestDestroyedEntitiesRegistrationSystem::TestDestroyedEntitiesRegistrationSystem(WorldHolder& worldHolder, DestroyedEntitiesTestCheck& testCheck)
	: mWorldHolder(worldHolder)
	, mTestCheck(testCheck)
{
}

void TestDestroyedEntitiesRegistrationSystem::update()
{
	World& world = mWorldHolder.getWorld();

	int count = 0;
	world.getSpatialData().getAllCellManagers().forEachComponentSet<DeathComponent>([&count](DeathComponent*)
	{
		++count;
	});

	mTestCheck.addDestroyedEntities(count);
}

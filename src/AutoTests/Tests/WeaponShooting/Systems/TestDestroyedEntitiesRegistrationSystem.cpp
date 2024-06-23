#include "Base/precomp.h"

#include "AutoTests/Tests/WeaponShooting/Systems/TestDestroyedEntitiesRegistrationSystem.h"

#include "GameData/Components/DeathComponent.generated.h"
#include "GameData/World.h"

#include "GameLogic/SharedManagers/WorldHolder.h"

#include "AutoTests/Tests/WeaponShooting/DestroyedEntitiesTestCheck.h"

TestDestroyedEntitiesRegistrationSystem::TestDestroyedEntitiesRegistrationSystem(
		WorldHolder& worldHolder,
		DestroyedEntitiesTestCheck& testCheck) noexcept
	: mWorldHolder(worldHolder)
	, mTestCheck(testCheck)
{
}

void TestDestroyedEntitiesRegistrationSystem::update()
{
	World& world = mWorldHolder.getWorld();

	int count = 0;
	world.getSpatialData().getAllCellManagers().forEachComponentSet<const DeathComponent>([&count](const DeathComponent*)
	{
		++count;
	});

	mTestCheck.addDestroyedEntities(count);
}

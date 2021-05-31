#include "Base/precomp.h"

#include "AutoTests/Tests/WeaponShooting/Systems/TestDestroyedEntitiesRegistrationSystem.h"

#include "Base/Random/Random.h"

#include "GameData/Components/DeathComponent.generated.h"
#include "GameData/World.h"

TestDestroyedEntitiesRegistrationSystem::TestDestroyedEntitiesRegistrationSystem(
		RaccoonEcs::ComponentFilter<const DeathComponent>&& deathFilter,
		WorldHolder& worldHolder,
		DestroyedEntitiesTestCheck& testCheck) noexcept
	: mDeathFilter(std::move(deathFilter))
	, mWorldHolder(worldHolder)
	, mTestCheck(testCheck)
{
}

void TestDestroyedEntitiesRegistrationSystem::update()
{
	World& world = mWorldHolder.getWorld();

	int count = 0;
	world.getSpatialData().getAllCellManagers().forEachComponentSetN(mDeathFilter, [&count](const DeathComponent*)
	{
		++count;
	});

	mTestCheck.addDestroyedEntities(count);
}

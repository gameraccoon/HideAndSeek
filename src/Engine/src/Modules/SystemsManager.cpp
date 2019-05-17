#include "Modules/SystemsManager.h"

#include <Debug/Log.h>

void SystemsManager::update(World* world, float dt)
{
	for (std::unique_ptr<System>& system : mSystems)
	{
		system->update(world, dt);
	}
}
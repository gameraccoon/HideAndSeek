#include "StealthGame.h"

#include <memory>

#include "Base/Engine.h"

#include <Modules/WorldLoader.h>

#include <Systems/RenderSystem.h>
#include <Systems/ControlSystem.h>
#include <Systems/CollisionSystem.h>
#include <Systems/ResourceStreamingSystem.h>

#include <Components/TransformComponent.h>
#include <Components/RenderComponent.h>
#include <Components/CollisionComponent.h>
#include <Components/MovementComponent.h>
#include <Components/CameraComponent.h>
#include <Components/LightComponent.h>

namespace Game
{
	void StealthGame::start()
	{
		mSystemsManager.registerSystem<ControlSystem>(getEngine(), &mKeyStates);
		mSystemsManager.registerSystem<CollisionSystem>();
		mSystemsManager.registerSystem<RenderSystem>(getEngine(), getResourceManager());
		mSystemsManager.registerSystem<ResourceStreamingSystem>(getResourceManager());

		mComponentFactory.registerComponent<TransformComponent>();
		mComponentFactory.registerComponent<RenderComponent>();
		mComponentFactory.registerComponent<CollisionComponent>();
		mComponentFactory.registerComponent<MovementComponent>();
		mComponentFactory.registerComponent<CameraComponent>();
		mComponentFactory.registerComponent<LightComponent>();

		WorldLoader::LoadWorld(mWorld, "test", mComponentFactory);

		// start the main loop
		getEngine()->start(this);
	}

	void StealthGame::setKeyState(int key, bool isPressed)
	{
		mKeyStates[key] = isPressed;
	}

	void StealthGame::update(float dt)
	{
		mSystemsManager.update(&mWorld, dt);
	}
}

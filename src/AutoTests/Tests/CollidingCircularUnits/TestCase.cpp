#include "Base/precomp.h"

#include "AutoTests/Tests/CollidingCircularUnits/TestCase.h"

#include <memory>

#include "HAL/Base/Engine.h"

#include "GameData/Spatial/SpatialWorldData.h"

#include "GameData/Components/SpriteCreatorComponent.generated.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/StateMachineComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"

#include "GameLogic/Systems/RenderSystem.h"
#include "GameLogic/Systems/CollisionSystem.h"
#include "GameLogic/Systems/ResourceStreamingSystem.h"
#include "GameLogic/Systems/MovementSystem.h"
#include "GameLogic/Systems/CharacterStateSystem.h"
#include "GameLogic/Systems/CameraSystem.h"

#include "AutoTests/Tests/CollidingCircularUnits/Systems/TestCircularUnitsSystem.h"
#include "AutoTests/Tests/CollidingCircularUnits/Systems/TestUnitsCountControlSystem.h"

void CollidingCircularUnitsTestCase::initTestCase(const ArgumentsParser& arguments)
{
	getResourceManager().loadAtlasesData("resources/atlas/atlas-list.json");

	getSystemsManager().registerSystem<TestUnitsCountControlSystem>(getWorldHolder());
	getSystemsManager().registerSystem<TestCircularUnitsSystem>(getWorldHolder(), getTime());
	getSystemsManager().registerSystem<CollisionSystem>(getWorldHolder());
	getSystemsManager().registerSystem<CameraSystem>(getWorldHolder(), getInputData());
	getSystemsManager().registerSystem<MovementSystem>(getWorldHolder(), getTime());
	getSystemsManager().registerSystem<CharacterStateSystem>(getWorldHolder(), getTime());
	getSystemsManager().registerSystem<ResourceStreamingSystem>(getWorldHolder(), getResourceManager());
	getSystemsManager().registerSystem<RenderSystem>(getWorldHolder(), getTime(), getResourceManager(), getThreadPool());

	Vector2D playerPos{ZERO_VECTOR};

	EntityView playerEntity = getWorldHolder().getWorld().createTrackedSpatialEntity(STR_TO_ID("ControlledEntity"), SpatialWorldData::GetCellForPos(playerPos));

	{
		TransformComponent* transform = playerEntity.addComponent<TransformComponent>();
		transform->setLocation(playerPos);
	}
	{
		SpriteCreatorComponent* sprite = playerEntity.addComponent<SpriteCreatorComponent>();
		SpriteDescription spriteDesc;
		spriteDesc.params.size = Vector2D(30.0f, 30.0f);
		spriteDesc.path = "resources/textures/hero.png";
		sprite->getDescriptionsRef().emplace_back(std::move(spriteDesc));
	}
	{
		CollisionComponent* collision = playerEntity.addComponent<CollisionComponent>();
		Hull& hull = collision->getGeometryRef();
		hull.type = HullType::Circular;
		hull.setRadius(15.0f);
	}
	playerEntity.addComponent<MovementComponent>();

	Vector2D cameraPos{ZERO_VECTOR};
	EntityView camera = getWorldHolder().getWorld().createTrackedSpatialEntity(STR_TO_ID("CameraEntity"), SpatialWorldData::GetCellForPos(cameraPos));

	{
		TransformComponent* transform = camera.addComponent<TransformComponent>();
		transform->setLocation(cameraPos);
	}
	camera.addComponent<MovementComponent>();

	startGame(arguments);
}

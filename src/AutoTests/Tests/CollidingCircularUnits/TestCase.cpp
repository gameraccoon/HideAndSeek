#include "EngineCommon/precomp.h"

#include "AutoTests/Tests/CollidingCircularUnits/TestCase.h"

#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"
#include "GameData/Components/SpriteCreatorComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Spatial/SpatialWorldData.h"

#include "GameUtils/ResourceManagement/ResourceManager.h"

#include "GameLogic/Systems/CameraSystem.h"
#include "GameLogic/Systems/CharacterStateSystem.h"
#include "GameLogic/Systems/CollisionSystem.h"
#include "GameLogic/Systems/InputSystem.h"
#include "GameLogic/Systems/MovementSystem.h"
#include "GameLogic/Systems/RenderSystem.h"
#include "GameLogic/Systems/ResourceStreamingSystem.h"

#include "AutoTests/Tests/CollidingCircularUnits/Systems/TestCircularUnitsSystem.h"
#include "AutoTests/Tests/CollidingCircularUnits/Systems/TestUnitsCountControlSystem.h"

void CollidingCircularUnitsTestCase::initTestCase(const ArgumentsParser& /*arguments*/)
{
	getResourceManager().loadAtlasesData(RelativeResourcePath("resources/atlas/atlas-list.json"));

	getGameLogicSystemsManager().registerSystem<InputSystem>(getWorldHolder(), getInputData());
	getGameLogicSystemsManager().registerSystem<TestUnitsCountControlSystem>(getWorldHolder());
	getGameLogicSystemsManager().registerSystem<TestCircularUnitsSystem>(getWorldHolder());
	getGameLogicSystemsManager().registerSystem<CollisionSystem>(getWorldHolder());
	getGameLogicSystemsManager().registerSystem<CameraSystem>(getWorldHolder());
	getGameLogicSystemsManager().registerSystem<MovementSystem>(getWorldHolder());
	getGameLogicSystemsManager().registerSystem<CharacterStateSystem>(getWorldHolder());
	getGameLogicSystemsManager().registerSystem<ResourceStreamingSystem>(getWorldHolder(), getResourceManager());
	getGameLogicSystemsManager().registerSystem<RenderSystem>(getWorldHolder(), getResourceManager(), getThreadPool());

	Vector2D playerPos{ ZERO_VECTOR };

	EntityView playerEntity = getWorldHolder().getWorld().createTrackedSpatialEntity(STR_TO_ID("ControlledEntity"), SpatialWorldData::GetCellForPos(playerPos));

	{
		TransformComponent* transform = playerEntity.addComponent<TransformComponent>();
		transform->setLocation(playerPos);
	}
	{
		SpriteCreatorComponent* sprite = playerEntity.addComponent<SpriteCreatorComponent>();
		SpriteDescription spriteDesc;
		spriteDesc.params.size = Vector2D(30.0f, 30.0f);
		spriteDesc.path = RelativeResourcePath("resources/textures/hero.png");
		sprite->getDescriptionsRef().emplace_back(std::move(spriteDesc));
	}
	{
		CollisionComponent* collision = playerEntity.addComponent<CollisionComponent>();
		Hull& hull = collision->getGeometryRef();
		hull.type = HullType::Circular;
		hull.setRadius(15.0f);
	}
	playerEntity.addComponent<MovementComponent>();

	Vector2D cameraPos{ ZERO_VECTOR };
	EntityView camera = getWorldHolder().getWorld().createTrackedSpatialEntity(STR_TO_ID("CameraEntity"), SpatialWorldData::GetCellForPos(cameraPos));

	{
		TransformComponent* transform = camera.addComponent<TransformComponent>();
		transform->setLocation(cameraPos);
	}
	camera.addComponent<MovementComponent>();
}

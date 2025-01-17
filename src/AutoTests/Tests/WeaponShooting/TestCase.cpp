#include "EngineCommon/precomp.h"

#include "AutoTests/Tests/WeaponShooting/TestCase.h"

#include "GameData/Components/CharacterStateComponent.generated.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"
#include "GameData/Components/SpriteCreatorComponent.generated.h"
#include "GameData/Components/StateMachineComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/WeaponComponent.generated.h"
#include "GameData/Spatial/SpatialWorldData.h"

#include "EngineUtils/ResourceManagement/ResourceManager.h"

#include "GameLogic/Systems/CameraSystem.h"
#include "GameLogic/Systems/CharacterStateSystem.h"
#include "GameLogic/Systems/CollisionSystem.h"
#include "GameLogic/Systems/DeadEntitiesDestructionSystem.h"
#include "GameLogic/Systems/InputSystem.h"
#include "GameLogic/Systems/MovementSystem.h"
#include "GameLogic/Systems/RenderSystem.h"
#include "GameLogic/Systems/ResourceStreamingSystem.h"
#include "GameLogic/Systems/WeaponSystem.h"

#include "AutoTests/Tests/WeaponShooting/Systems/TestDestroyedEntitiesRegistrationSystem.h"
#include "AutoTests/Tests/WeaponShooting/Systems/TestShootingControlSystem.h"
#include "AutoTests/Tests/WeaponShooting/Systems/TestSpawnShootableUnitsSystem.h"

void WeaponShootingTestCase::initTestCase(const ArgumentsParser& /*arguments*/)
{
	getResourceManager().loadAtlasesData(RelativeResourcePath("resources/atlas/atlas-list.json"));

	DestroyedEntitiesTestCheck& destroyedEntitiesTestCheck = mTestChecklist.addCheck<DestroyedEntitiesTestCheck>(100);

	getGameLogicSystemsManager().registerSystem<InputSystem>(getWorldHolder(), getInputData());
	getGameLogicSystemsManager().registerSystem<TestSpawnShootableUnitsSystem>(getWorldHolder());
	getGameLogicSystemsManager().registerSystem<TestShootingControlSystem>(getWorldHolder());
	getGameLogicSystemsManager().registerSystem<CollisionSystem>(getWorldHolder());
	getGameLogicSystemsManager().registerSystem<CameraSystem>(getWorldHolder());
	getGameLogicSystemsManager().registerSystem<MovementSystem>(getWorldHolder());
	getGameLogicSystemsManager().registerSystem<CharacterStateSystem>(getWorldHolder());
	getGameLogicSystemsManager().registerSystem<WeaponSystem>(getWorldHolder());
	getGameLogicSystemsManager().registerSystem<TestDestroyedEntitiesRegistrationSystem>(getWorldHolder(), destroyedEntitiesTestCheck);
	getGameLogicSystemsManager().registerSystem<DeadEntitiesDestructionSystem>(getWorldHolder());
	getGameLogicSystemsManager().registerSystem<ResourceStreamingSystem>(getWorldHolder(), getResourceManager());
	getGameLogicSystemsManager().registerSystem<RenderSystem>(getWorldHolder(), getResourceManager(), getThreadPool());

	World& world = getWorldHolder().getWorld();

	Vector2D playerPos{ ZERO_VECTOR };
	EntityView playerEntity = world.createTrackedSpatialEntity(STR_TO_ID("ControlledEntity"), SpatialWorldData::GetCellForPos(playerPos));

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
	{
		WeaponComponent* weapon = playerEntity.addComponent<WeaponComponent>();
		weapon->setShotDistance(1000.0f);
		weapon->setDamageValue(70.0f);
		weapon->setShotPeriod(0.0001f);
	}
	playerEntity.addComponent<CharacterStateComponent>();

	Vector2D cameraPos{ ZERO_VECTOR };
	EntityView camera = world.createTrackedSpatialEntity(STR_TO_ID("CameraEntity"), SpatialWorldData::GetCellForPos(cameraPos));

	{
		TransformComponent* transform = camera.addComponent<TransformComponent>();
		transform->setLocation(cameraPos);
	}
	camera.addComponent<MovementComponent>();

	mTicksToFinish = 300;
}

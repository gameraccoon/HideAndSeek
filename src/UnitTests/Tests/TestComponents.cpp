#include "Base/precomp.h"

#include <gtest/gtest.h>

#include "Base/Types/TemplateAliases.h"

#include "GameData/EcsDefinitions.h"
#include "GameData/ComponentRegistration/ComponentFactoryRegistration.h"
#include "GameData/Components/TransformComponent.generated.h"

TEST(Components, EntityCreationAndRemovement)
{
	ComponentFactory componentFactory;
	RaccoonEcs::EntityGenerator entityGenerator(42);
	ComponentsRegistration::RegisterComponents(componentFactory);

	EntityManager entityManager(componentFactory, entityGenerator);
	Entity testEntity1 = entityManager.addEntity();
	Entity testEntity2 = entityManager.addEntity();

	EXPECT_NE(testEntity1, testEntity2);
	EXPECT_NE(testEntity1.getId(), testEntity2.getId());

	entityManager.removeEntity(testEntity2);

	Entity testEntity3 = entityManager.addEntity();

	EXPECT_NE(testEntity1, testEntity3);
	EXPECT_NE(testEntity1.getId(), testEntity3.getId());
}

TEST(Components, ComponentsAttachment)
{
	ComponentFactory componentFactory;
	RaccoonEcs::EntityGenerator entityGenerator(42);
	ComponentsRegistration::RegisterComponents(componentFactory);

	Vector2D location(Vector2D(1.0f, 0.0f));

	EntityManager entityManager(componentFactory, entityGenerator);
	Entity testEntity = entityManager.addEntity();
	TransformComponent* transform = entityManager.addComponent<TransformComponent>(testEntity);
	transform->setLocation(location);

	auto [resultTransform] = entityManager.getEntityComponents<TransformComponent>(testEntity);

	EXPECT_NE(nullptr, transform);
	EXPECT_TRUE(location == resultTransform->getLocation());
}

TEST(Components, RemoveEntityWithComponents)
{
	ComponentFactory componentFactory;
	RaccoonEcs::EntityGenerator entityGenerator(42);
	ComponentsRegistration::RegisterComponents(componentFactory);

	Vector2D location1(Vector2D(1.0f, 0.0f));
	Vector2D location2(Vector2D(0.0f, 1.0f));
	Vector2D location3(Vector2D(1.0f, 1.0f));

	EntityManager entityManager(componentFactory, entityGenerator);
	Entity testEntity1 = entityManager.addEntity();
	TransformComponent* transform1 = entityManager.addComponent<TransformComponent>(testEntity1);
	transform1->setLocation(location1);

	Entity testEntity2 = entityManager.addEntity();
	TransformComponent* transform2 = entityManager.addComponent<TransformComponent>(testEntity2);
	transform2->setLocation(location2);

	TupleVector<TransformComponent*> components;
	entityManager.getComponents<TransformComponent>(components);
	EXPECT_EQ(static_cast<size_t>(2u), components.size());

	bool location1Found = false;
	bool location2Found = false;
	for (auto& [transform] : components)
	{
		Vector2D location = transform->getLocation();
		if (location == location1 && location1Found == false)
		{
			location1Found = true;
		}
		else if (location == location2 && location2Found == false)
		{
			location2Found = true;
		}
		else
		{
			GTEST_FAIL();
		}
	}
	EXPECT_TRUE(location1Found);
	EXPECT_TRUE(location2Found);

	entityManager.removeEntity(testEntity2);

	Entity testEntity3 = entityManager.addEntity();
	TransformComponent* transform3 = entityManager.addComponent<TransformComponent>(testEntity3);
	transform3->setLocation(location3);

	location1Found = false;
	location2Found = false;
	bool location3Found = false;
	TupleVector<TransformComponent*> transforms;
	entityManager.getComponents<TransformComponent>(transforms);
	EXPECT_EQ(static_cast<size_t>(2u), transforms.size());
	for (auto& [transform] : transforms)
	{
		Vector2D location = transform->getLocation();
		if (location == location1 && !location1Found)
		{
			location1Found = true;
		}
		else if (location == location2 && !location2Found)
		{
			location2Found = true;
		}
		else if (location == location3 && !location3Found)
		{
			location3Found = true;
		}
		else
		{
			GTEST_FAIL();
		}
	}
	EXPECT_TRUE(location1Found);
	EXPECT_FALSE(location2Found);
	EXPECT_TRUE(location3Found);

	entityManager.clearCaches();
	transforms.clear();
	entityManager.getComponents<TransformComponent>(transforms);
	EXPECT_EQ(static_cast<size_t>(2u), transforms.size());

	entityManager.removeEntity(testEntity3);
	entityManager.clearCaches();
	transforms.clear();
	entityManager.getComponents<TransformComponent>(transforms);
	EXPECT_EQ(static_cast<size_t>(1u), transforms.size());
}

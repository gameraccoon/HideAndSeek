#include "Base/precomp.h"

#include <gtest/gtest.h>

#include "ECS/EntityManager.h"
#include "GameData/Components/TransformComponent.generated.h"

TEST(Components, EntityCreationAndRemovement)
{
	EntityManager entityManager;
	Entity testEntity1 = entityManager.addEntity();
	Entity testEntity2 = entityManager.addEntity();

	EXPECT_NE(testEntity1, testEntity2);
	EXPECT_NE(testEntity1.getID(), testEntity2.getID());

	entityManager.removeEntity(testEntity2);

	Entity testEntity3 = entityManager.addEntity();

	EXPECT_NE(testEntity1, testEntity3);
	EXPECT_NE(testEntity1.getID(), testEntity3.getID());
}

TEST(Components, ComponentsAttachment)
{
	Vector2D location(Vector2D(1.0f, 0.0f));

	EntityManager entityManager;
	Entity testEntity = entityManager.addEntity();
	TransformComponent* transform = entityManager.addComponent<TransformComponent>(testEntity);
	transform->setLocation(location);

	auto [resultTransform] = entityManager.getEntityComponents<TransformComponent>(testEntity);

	EXPECT_NE(nullptr, transform);
	EXPECT_TRUE(location == resultTransform->getLocation());
}

TEST(Components, RemoveEntityWithComponents)
{
	Vector2D location1(Vector2D(1.0f, 0.0f));
	Vector2D location2(Vector2D(0.0f, 1.0f));
	Vector2D location3(Vector2D(1.0f, 1.0f));

	EntityManager entityManager;
	Entity testEntity1 = entityManager.addEntity();
	TransformComponent* transform1 = entityManager.addComponent<TransformComponent>(testEntity1);
	transform1->setLocation(location1);

	Entity testEntity2 = entityManager.addEntity();
	TransformComponent* transform2 = entityManager.addComponent<TransformComponent>(testEntity2);
	transform2->setLocation(location2);

	TupleVector<TransformComponent*> components;
	entityManager.getComponents<TransformComponent>(components);
	EXPECT_EQ(static_cast<size_t>(2), components.size());

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
	for (auto& [transform] : transforms)
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
		else if (location == location3 && location3Found == false)
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
}

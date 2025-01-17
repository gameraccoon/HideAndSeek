#include "EngineCommon/precomp.h"

#include <gtest/gtest.h>

#include "GameData/ComponentRegistration/ComponentFactoryRegistration.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/World.h"

#include "GameUtils/Geometry/Collide.h"
#include "GameUtils/Geometry/RayTrace.h"

struct CollidableObjects
{
	OptionalEntity rect;
	OptionalEntity circle;
};

CollidableObjects FillCollidableObjects(World& world)
{
	CollidableObjects result;

	{
		Vector2D rectPos(50.0f, 30.0f);
		WorldCell& cell = world.getSpatialData().getOrCreateCell(SpatialWorldData::GetCellForPos(rectPos));
		EntityManager& cellEntityManager = cell.getEntityManager();
		const Entity entity = cellEntityManager.addEntity();
		TransformComponent* transform = cellEntityManager.addComponent<TransformComponent>(entity);
		transform->setLocation(rectPos);
		transform->setRotation(Rotator(0.0f));
		CollisionComponent* collision = cellEntityManager.addComponent<CollisionComponent>(entity);
		Hull geometry;
		geometry.type = HullType::Angular;
		geometry.points = { { -10.0f, -10.0f }, { 10.0f, -10.0f }, { 10.0f, 10.0f }, { -10.0f, 10.0f } };
		geometry.generateBorders();
		collision->setGeometry(geometry);
		Collide::UpdateBoundingBox(collision);
		result.rect = entity;
	}

	{
		Vector2D circlePos(550.0f, 30.0f);
		WorldCell& cell = world.getSpatialData().getOrCreateCell(SpatialWorldData::GetCellForPos(circlePos));
		EntityManager& cellEntityManager = cell.getEntityManager();
		const Entity entity = cellEntityManager.addEntity();
		TransformComponent* transform = cellEntityManager.addComponent<TransformComponent>(entity);
		transform->setLocation(circlePos);
		transform->setRotation(Rotator(0.0f));
		CollisionComponent* collision = cellEntityManager.addComponent<CollisionComponent>(entity);
		Hull geometry;
		geometry.type = HullType::Circular;
		geometry.setRadius(10.0f);
		collision->setGeometry(geometry);
		Collide::UpdateBoundingBox(collision);
		result.circle = entity;
	}

	return result;
}

TEST(Raytrace, FastTraceRect1)
{
	ComponentFactory componentFactory;
	ComponentsRegistration::RegisterComponents(componentFactory);
	World world(componentFactory);
	FillCollidableObjects(world);

	EXPECT_TRUE(RayTrace::FastTrace(world, Vector2D(20.f, 20.f), Vector2D(80.f, 60.f))); // out-pierce-out

	EXPECT_TRUE(RayTrace::FastTrace(world, Vector2D(30.f, 15.f), Vector2D(55.f, 35.f))); // out-in

	EXPECT_FALSE(RayTrace::FastTrace(world, Vector2D(55.f, 35.f), Vector2D(80.f, 60.f))); // in-out

	EXPECT_FALSE(RayTrace::FastTrace(world, Vector2D(80.f, 35.f), Vector2D(80.f, 60.f))); // side

	EXPECT_FALSE(RayTrace::FastTrace(world, Vector2D(20.f, -3.f), Vector2D(80.f, 10.f))); // side
}

TEST(Raytrace, FastTraceRect2)
{
	ComponentFactory componentFactory;
	ComponentsRegistration::RegisterComponents(componentFactory);
	World world(componentFactory);
	FillCollidableObjects(world);

	EXPECT_TRUE(RayTrace::FastTrace(world, Vector2D(35.f, 15.f), Vector2D(65.f, 45.f)));

	EXPECT_TRUE(RayTrace::FastTrace(world, Vector2D(20.f, 60.f), Vector2D(80.f, 0.f)));
}

TEST(Raytrace, TraceRect)
{
	ComponentFactory componentFactory;
	ComponentsRegistration::RegisterComponents(componentFactory);
	World world(componentFactory);

	const CollidableObjects objects = FillCollidableObjects(world);

	const RayTrace::TraceResult traceResult = RayTrace::Trace(world, Vector2D(20.f, 20.f), Vector2D(80.f, 60.f));

	EXPECT_TRUE(traceResult.hasHit);
	EXPECT_EQ(objects.rect.getEntity(), traceResult.hitEntity.entity.getEntity());
}

TEST(Raytrace, FastTraceCircle)
{
	ComponentFactory componentFactory;
	ComponentsRegistration::RegisterComponents(componentFactory);
	World world(componentFactory);
	FillCollidableObjects(world);

	EXPECT_TRUE(RayTrace::FastTrace(world, Vector2D(520.f, 20.f), Vector2D(580.f, 60.f))); // out-pierce-out

	EXPECT_TRUE(RayTrace::FastTrace(world, Vector2D(530.f, 15.f), Vector2D(555.f, 35.f))); // out-in

	EXPECT_FALSE(RayTrace::FastTrace(world, Vector2D(555.f, 35.f), Vector2D(580.f, 60.f))); // in-out

	EXPECT_FALSE(RayTrace::FastTrace(world, Vector2D(580.f, 35.f), Vector2D(580.f, 60.f))); // side

	EXPECT_FALSE(RayTrace::FastTrace(world, Vector2D(520.f, -3.f), Vector2D(580.f, 10.f))); // side

	EXPECT_TRUE(RayTrace::FastTrace(world, Vector2D(535.f, 15.f), Vector2D(565.f, 45.f)));

	EXPECT_TRUE(RayTrace::FastTrace(world, Vector2D(520.f, 60.f), Vector2D(580.f, 0.f)));
}

TEST(Raytrace, TraceCircle)
{
	ComponentFactory componentFactory;
	ComponentsRegistration::RegisterComponents(componentFactory);
	World world(componentFactory);

	const CollidableObjects objects = FillCollidableObjects(world);

	const RayTrace::TraceResult traceResult = RayTrace::Trace(world, Vector2D(520.f, 20.f), Vector2D(580.f, 60.f));

	EXPECT_TRUE(traceResult.hasHit);
	EXPECT_EQ(objects.circle.getEntity(), traceResult.hitEntity.entity.getEntity());
}

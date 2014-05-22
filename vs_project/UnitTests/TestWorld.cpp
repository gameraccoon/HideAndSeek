#include "stdafx.h"
#include "CppUnitTest.h"

#include "../../src/Engine/Core/IActor.h"

#include <Engine/Core/World.h>
#include <Engine/Actors/Actor.h>
#include <Engine/Structures/Hull.h>
#include <Engine/Structures/BoundingBox.h>
#include <Engine/Structures/Border.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestWorld
{
	int TestValue;
	int DestructionFlag;

	TEST_CLASS(TestWorld)
	{
	private:
		class TestActor : public Actor
		{
		public:
			TestActor(World * world) : Actor(world, ZERO_VECTOR, Rotator(0.f)) { }
			
			~TestActor()
			{
				DestructionFlag += 1;
			}

			virtual void update(float deltaTime) override final
			{
				TestValue += (int)deltaTime;
			}
		
			void Render(Vector2D shift, Rotator angle) { }
		protected:
			virtual void hit(IActor *instigator, float damageValue, Vector2D impulse) override final { };
			virtual void updateCollision() override final { }
		};
	public:
		TEST_METHOD(TestWorldAddingStaticActor)
		{
			World *testWorld = new World();

			TestActor actor1(testWorld);

			TestValue = 0;

			testWorld->update(1.f);

			Assert::AreEqual(1, TestValue);

			delete testWorld;
		}

		TEST_METHOD(TestWorldDeletionStaticActor)
		{
			World *testWorld = new World();

			TestValue = 0;

			testWorld->update(1.f);

			Assert::AreEqual(0, TestValue);

			delete testWorld;
		}

		TEST_METHOD(TestWorldAdditionAndDeletionNonStaticActor)
		{
			World *testWorld = new World();

			IActor *actor1 = new TestActor(testWorld);

			TestValue = 0;

			testWorld->update(1.f);

			Assert::AreEqual(1, TestValue);

			actor1->destroy();

			TestValue = 0;

			testWorld->update(1.f);

			Assert::AreEqual(0, TestValue);

			delete testWorld;
		}

		TEST_METHOD(TestWorldCleaningUp)
		{
			World *testWorld = new World();

			IActor *actor1 = new TestActor(testWorld);

			IActor *actor2 = new TestActor(testWorld);

			DestructionFlag = 0;

			delete testWorld;

			Assert::AreEqual(2, DestructionFlag);
		}
	};
}
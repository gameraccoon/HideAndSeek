#pragma once

#include "Base/Debug/Assert.h"

class World;
class GameData;

class WorldHolder
{
public:
	WorldHolder(World* world, GameData& gameData)
		: mWorld(world)
		, mGameData(gameData)
	{
		AssertFatal(mWorld, "World is not set");
	}

	void setWorld(World* newWorld)
	{
		AssertFatal(newWorld, "World is not set");
		mWorld = newWorld;
	}

	World& getWorld()
	{
		return *mWorld;
	}

	GameData& getGameData()
	{
		return mGameData;
	}

private:
	World* mWorld;
	GameData& mGameData;
};

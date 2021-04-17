#pragma once

#include "GameData/Core/Vector2D.h"
#include "GameData/Time/GameplayTimestamp.h"

struct DebugDrawPrimitive
{
	GameplayTimestamp lifeDeadline;

	explicit DebugDrawPrimitive(GameplayTimestamp lifeDeadline)
		: lifeDeadline(lifeDeadline)
	{
	}

	[[nodiscard]] bool isLifeTimeExceeded(GameplayTimestamp now) const
	{
		return now > lifeDeadline;
	}
};

struct DebugDrawWorldPoint : DebugDrawPrimitive
{
	Vector2D pos;
	std::string name;

	DebugDrawWorldPoint(const Vector2D& pos, GameplayTimestamp lifeDeadline)
		: DebugDrawPrimitive(lifeDeadline)
		, pos(pos)
	{
	}

	DebugDrawWorldPoint(const Vector2D& pos, const std::string& name, GameplayTimestamp lifeDeadline)
		: DebugDrawPrimitive(lifeDeadline)
		, pos(pos)
		, name(name)
	{
	}
};

struct DebugDrawScreenPoint : DebugDrawPrimitive
{
	Vector2D screenPos;
	std::string name;
	GameplayTimestamp lifeDeadline;

	DebugDrawScreenPoint(const Vector2D& screenPos, GameplayTimestamp lifeDeadline)
		: DebugDrawPrimitive(lifeDeadline)
		, screenPos(screenPos)
	{
	}

	DebugDrawScreenPoint(const Vector2D& screenPos, const std::string& name, GameplayTimestamp lifeDeadline)
		: DebugDrawPrimitive(lifeDeadline)
		, screenPos(screenPos)
		, name(name)
	{
	}
};

struct DebugDrawWorldLineSegment : DebugDrawPrimitive
{
	Vector2D startPos;
	Vector2D endPos;
	GameplayTimestamp lifeDeadline;

	DebugDrawWorldLineSegment(const Vector2D& startPos, const Vector2D& endPos, GameplayTimestamp lifeDeadline)
		: DebugDrawPrimitive(lifeDeadline)
		, startPos(startPos)
		, endPos(endPos)
	{
	}
};

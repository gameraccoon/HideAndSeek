#pragma once

#include <array>

#include "GameData/Time/GameplayTimestamp.h"

namespace GameplayInput
{
	enum class InputAxis
	{
		MoveHorizontal = 0,
		MoveVertical,
		AimHorizontal,
		AimVertical,
		// add new elements above this line
		Count
	};

	enum class InputKey
	{
		Shoot = 0,
		Sprint,
		// add new elements above this line
		Count
	};

	class FrameState
	{
	public:
		void updateAxis(InputAxis axis, float newValue);
		float getAxisValue(InputAxis axis) const;

		void updateKey(InputKey key, bool isPressed, GameplayTimestamp currentTimestamp);
		bool isJustPressed(InputKey key) const;
		bool isJustReleased(InputKey key) const;
		bool isPressed(InputKey key) const;
		GameplayTimestamp getLastFlipTime(InputKey key) const;

		void setCurrentFrameTimestamp(GameplayTimestamp timestamp);

	private:
		struct KeyState
		{
			bool isPressed = false;
			GameplayTimestamp lastFlipTime;
		};

	private:
		std::array<float, static_cast<size_t>(InputAxis::Count)> mAxes;
		std::array<KeyState, static_cast<size_t>(InputKey::Count)> mKeys;
		GameplayTimestamp mCurrentFrameTimestamp;
	};
}

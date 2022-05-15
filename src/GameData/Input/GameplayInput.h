#pragma once

#include <array>

#include "GameData/Time/GameplayTimestamp.h"

namespace GameplayInput
{
	enum class KeyState
	{
		Inactive = 0,
		JustActivated = 1,
		Active = 2,
		JustDeactivated = 3,
	};

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

		void updateKey(InputKey key, KeyState newState, GameplayTimestamp currentTimestamp);
		KeyState getKeyState(InputKey key) const;
		bool isKeyJustActivated(InputKey key) const;
		bool isKeyActive(InputKey key) const;
		bool isKeyJustDeactivated(InputKey key) const;
		GameplayTimestamp getLastFlipTime(InputKey key) const;

	private:
		struct KeyInfo
		{
			KeyState state = KeyState::Inactive;
			GameplayTimestamp lastFlipTime;
		};

	private:
		std::array<float, static_cast<size_t>(InputAxis::Count)> mAxes{};
		std::array<KeyInfo, static_cast<size_t>(InputKey::Count)> mKeys;
	};
}

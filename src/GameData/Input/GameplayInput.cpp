#include "Base/precomp.h"

#include "GameData/Input/GameplayInput.h"

namespace GameplayInput
{
	void FrameState::updateAxis(InputAxis axis, float newValue)
	{
		mAxes[static_cast<size_t>(axis)] = newValue;
	}

	float FrameState::getAxisValue(InputAxis axis) const
	{
		return mAxes[static_cast<size_t>(axis)];
	}

	void FrameState::updateKey(InputKey key, bool isPressed, GameplayTimestamp currentTimestamp)
	{
		KeyState& state = mKeys[static_cast<size_t>(key)];
		if (state.isPressed != isPressed)
		{
			state.isPressed = isPressed;
			state.lastFlipTime = currentTimestamp;
		}
	}

	bool FrameState::isJustPressed(InputKey key) const
	{
		return mKeys[static_cast<size_t>(key)].isPressed && mKeys[static_cast<size_t>(key)].lastFlipTime == mCurrentFrameTimestamp;
	}

	bool FrameState::isJustReleased(InputKey key) const
	{
		return !mKeys[static_cast<size_t>(key)].isPressed && mKeys[static_cast<size_t>(key)].lastFlipTime == mCurrentFrameTimestamp;
	}

	bool FrameState::isPressed(InputKey key) const
	{
		return mKeys[static_cast<size_t>(key)].isPressed;
	}

	GameplayTimestamp FrameState::getLastFlipTime(InputKey key) const
	{
		return mKeys[static_cast<size_t>(key)].lastFlipTime;
	}

	void FrameState::setCurrentFrameTimestamp(GameplayTimestamp timestamp)
	{
		mCurrentFrameTimestamp = timestamp;
	}
}

static_assert(std::is_trivially_copyable<GameplayInput::FrameState>(), "GameplayInput should be trivially copyable");

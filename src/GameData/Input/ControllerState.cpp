#include "Base/precomp.h"

#include "GameData/Input/ControllerState.h"

namespace Input
{
	void ControllerState::updateButtonState(int button, bool isPressed)
	{
		if (isPressed && mPressedButtons.find(button) == mPressedButtons.end())
		{
			mPressedButtons.insert(button);
			mLastFramePressedButtons.push_back(button);
		}
		else if (!isPressed)
		{
			mPressedButtons.erase(button);
			mLastFrameReleasedButtons.push_back(button);
		}
	}

	void ControllerState::clearLastFrameState()
	{
		mLastFramePressedButtons.clear();
		mLastFrameReleasedButtons.clear();
	}

	bool ControllerState::isButtonPressed(int button) const
	{
		return mPressedButtons.find(button) != mPressedButtons.end();
	}

	bool ControllerState::isButtonJustPressed(int button) const
	{
		return std::find(mLastFramePressedButtons.begin(), mLastFramePressedButtons.end(), button) != mLastFramePressedButtons.end();
	}

	bool ControllerState::isButtonJustReleased(int button) const
	{
		return std::find(mLastFrameReleasedButtons.begin(), mLastFrameReleasedButtons.end(), button) != mLastFrameReleasedButtons.end();
	}

	void ControllerState::updateAxis(int axis, float newValue)
	{
		mAxes[axis] = newValue;
	}

	float ControllerState::getAxisValue(int axis) const
	{
		const auto it = mAxes.find(axis);
		if (it != mAxes.end())
		{
			return it->second;
		}
		else
		{
			return 0.0f;
		}
	}
}

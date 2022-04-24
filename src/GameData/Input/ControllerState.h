#pragma once

#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace Input
{
	enum class ControllerType
	{
		Keyboard,
		Mouse,
		Gamepad
	};

	class ControllerState
	{
	public:
		void clearLastFrameState();

		void updateButtonState(int button, bool isPressed);
		bool isButtonPressed(int button) const;
		bool isButtonJustPressed(int button) const;
		bool isButtonJustReleased(int button) const;

		void updateAxis(int axis, float newValue);
		float getAxisValue(int axis) const;

	private:
		std::unordered_set<int> mPressedButtons;
		std::vector<int> mLastFramePressedButtons;
		std::vector<int> mLastFrameReleasedButtons;

		std::unordered_map<int, float> mAxes;
	};

	using PlayerControllerStates = std::unordered_map<ControllerType, ControllerState>;
}

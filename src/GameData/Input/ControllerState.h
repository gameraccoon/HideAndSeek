#pragma once

#include <array>
#include <bitset>

namespace Input
{
	class BaseControllerState
	{
	public:
		virtual ~BaseControllerState() noexcept = default;

		virtual void clearLastFrameState() noexcept = 0;

		virtual void updateButtonState(size_t button, bool isPressed) noexcept = 0;
		virtual bool isButtonPressed(size_t button) const noexcept = 0;
		virtual bool isButtonJustPressed(size_t button) const noexcept = 0;
		virtual bool isButtonJustReleased(size_t button) const noexcept = 0;

		virtual void updateAxis(size_t axis, float newValue) noexcept = 0;
		virtual float getAxisValue(size_t axis) const noexcept = 0;
	};

	template<size_t HardwareButtonsCount, size_t HardwareAxesCount>
	class ControllerState final : public BaseControllerState
	{
	public:
		void clearLastFrameState() noexcept final
		{
			mLastFramePressedButtons.reset();
			mLastFrameReleasedButtons.reset();
		}

		void updateButtonState(size_t button, bool isPressed) noexcept final
		{
			AssertFatal(button < HardwareButtonsCount, "Invalid button index %d, max is %d", button, HardwareButtonsCount);
			const bool wasPressed = mPressedButtons.test(button);
			if (isPressed)
			{
				mPressedButtons.set(button, true);
				if (!wasPressed)
				{
					mLastFramePressedButtons.set(button, true);
				}
			}
			else if (!isPressed)
			{
				mPressedButtons.set(button, false);
				if (wasPressed)
				{
					mLastFrameReleasedButtons.set(button, true);
				}
			}
		}

		bool isButtonPressed(size_t button) const noexcept final
		{
			AssertFatal(button < HardwareButtonsCount, "Invalid button index %d max is %d", button, HardwareButtonsCount);
			return mPressedButtons.test(button);
		}

		bool isButtonJustPressed(size_t button) const noexcept final
		{
			AssertFatal(button < HardwareButtonsCount, "Invalid button index %d max is %d", button, HardwareButtonsCount);
			return mLastFramePressedButtons.test(button);
		}

		bool isButtonJustReleased(size_t button) const noexcept final
		{
			AssertFatal(button < HardwareButtonsCount, "Invalid button index %d max is %d", button, HardwareButtonsCount);
			return mLastFrameReleasedButtons.test(button);
		}

		void updateAxis(size_t axis, float newValue) noexcept final
		{
			AssertFatal(axis < HardwareAxesCount, "Invalid axis index %d max is %d", axis, HardwareAxesCount);
			mAxes[axis] = newValue;
		}

		float getAxisValue(size_t axis) const noexcept final
		{
			AssertFatal(axis < HardwareAxesCount, "Invalid axis index %d max is %d", axis, HardwareAxesCount);
			return mAxes[axis];
		}

	private:
		std::bitset<HardwareButtonsCount> mPressedButtons;
		std::bitset<HardwareButtonsCount> mLastFramePressedButtons;
		std::bitset<HardwareButtonsCount> mLastFrameReleasedButtons;

		std::array<float, HardwareAxesCount> mAxes;
	};
}

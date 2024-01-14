#pragma once

#include <array>

#include "Base/Types/ComplexTypes/SimpleBitset.h"

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
			mLastFramePressedButtons.clear();
			mLastFrameReleasedButtons.clear();
		}

		void updateButtonState(size_t button, bool isPressed) noexcept final
		{
			AssertFatal(button < HardwareButtonsCount, "Invalid button index %d, max is %d", button, HardwareButtonsCount);
			const bool wasPressed = mPressedButtons.get(button);
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
			return mPressedButtons.get(button);
		}

		bool isButtonJustPressed(size_t button) const noexcept final
		{
			AssertFatal(button < HardwareButtonsCount, "Invalid button index %d max is %d", button, HardwareButtonsCount);
			return mLastFramePressedButtons.get(button);
		}

		bool isButtonJustReleased(size_t button) const noexcept final
		{
			AssertFatal(button < HardwareButtonsCount, "Invalid button index %d max is %d", button, HardwareButtonsCount);
			return mLastFrameReleasedButtons.get(button);
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

		const SimpleBitset<HardwareButtonsCount>& getPressedButtons() const noexcept
		{
			return mPressedButtons;
		}

		void updatePressedButtonsFromRawData(const std::array<std::byte, BitsetTraits<HardwareButtonsCount>::ByteCount>& pressedButtons) noexcept
		{

			SimpleBitset<HardwareButtonsCount> pressedButtonsThisFrame;
			pressedButtonsThisFrame.setRawData(pressedButtons.data());

			mLastFramePressedButtons = mPressedButtons;
			mLastFramePressedButtons.invert();
			mLastFramePressedButtons.intersect(pressedButtonsThisFrame);

			mLastFrameReleasedButtons = pressedButtonsThisFrame;
			mLastFrameReleasedButtons.invert();
			mLastFrameReleasedButtons.intersect(mPressedButtons);

			mPressedButtons = pressedButtonsThisFrame;
		}

	private:
		SimpleBitset<HardwareButtonsCount> mPressedButtons;
		SimpleBitset<HardwareButtonsCount> mLastFramePressedButtons;
		SimpleBitset<HardwareButtonsCount> mLastFrameReleasedButtons;

		std::array<float, HardwareAxesCount> mAxes = {};
	};
}

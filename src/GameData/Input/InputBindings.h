#pragma once

#include <vector>
#include <unordered_map>
#include <memory>

#include "GameData/Input/ControllerState.h"
#include "GameData/Input/GameplayInput.h"

namespace Input
{
	class KeyBinding
	{
	public:
		virtual ~KeyBinding() = default;
		virtual bool isActive(const PlayerControllerStates& controllerStates) const = 0;
	};

	class AxisBinding
	{
	public:
		virtual ~AxisBinding() = default;
		virtual float getAxisValue(const PlayerControllerStates& controllerStates) const = 0;
	};

	/**
	 * returns true if the button is pressed
	 */
	class PressSingleButtonKeyBinding final : public KeyBinding
	{
	public:
		PressSingleButtonKeyBinding(ControllerType controllerType, int button);
		bool isActive(const PlayerControllerStates& controllerStates) const override;

	private:
		const ControllerType mControllerType;
		const int mButton;
	};

	/**
	 * Processes a chord of buttons (can be one or zero button as well)
	 * returns false if expected chord is empty
	 * returns false if any of buttons in the chord are not pressed
	 * otherwice returns true
	 */
	class PressButtonChordKeyBinding final : public KeyBinding
	{
	public:
		PressButtonChordKeyBinding(ControllerType controllerType, const std::vector<int>& buttons);
		bool isActive(const PlayerControllerStates& controllerStates) const override;

	private:
		const ControllerType mControllerType;
		const std::vector<int> mButtons;
	};

	/**
	 * returns 1.0 when button is pressed, and 0.0 when it's released
	 */
	class PositiveButtonAxisBinding final : public AxisBinding
	{
	public:
		PositiveButtonAxisBinding(ControllerType controllerType, int button);
		virtual float getAxisValue(const PlayerControllerStates& controllerStates) const override;
	private:
		const ControllerType mControllerType;
		const int mButton;
	};

	/**
	 * returns -1.0 when button is pressed, and 0.0 when it's released
	 */
	class NegativeButtonAxisBinding final : public AxisBinding
	{
	public:
		NegativeButtonAxisBinding(ControllerType controllerType, int button);
		virtual float getAxisValue(const PlayerControllerStates& controllerStates) const override;
	private:
		const ControllerType mControllerType;
		const int mButton;
	};

	/**
	 * returns unchanged value of the controller axis
	 */
	class DirectAxisToAxisBinding final : public AxisBinding
	{
	public:
		DirectAxisToAxisBinding(ControllerType controllerType, int axis);
		virtual float getAxisValue(const PlayerControllerStates& controllerStates) const override;
	private:
		const ControllerType mControllerType;
		const int mAxis;
	};

	struct InputBindings
	{
		using KeyBindingList = std::vector<std::unique_ptr<KeyBinding>>;
		using AxisBindingList = std::vector<std::unique_ptr<AxisBinding>>;
		using KeyBindingMap = std::unordered_map<GameplayInput::InputKey, KeyBindingList>;
		using AxisBindingMap = std::unordered_map<GameplayInput::InputAxis, AxisBindingList>;

		KeyBindingMap keyBindings;
		AxisBindingMap axisBindings;

		/**
		 * return true if any of bindings associated to this key are pressed
		 */
		static bool IsKeyPressed(const std::vector<std::unique_ptr<KeyBinding>>& bindings, const PlayerControllerStates& controllerStates);

		/**
		 * returns an average value from multiple bindings
		 * the axes with zero value don't participate in the calculation
		 */
		static float GetBlendedAxisValue(const std::vector<std::unique_ptr<AxisBinding>>& bindings, const PlayerControllerStates& controllerStates);
	};
}

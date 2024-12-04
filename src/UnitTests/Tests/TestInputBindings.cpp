#include "EngineCommon/precomp.h"

#include <gtest/gtest.h>

#include "EngineData/Input/InputBindingTypes.h"
#include "EngineData/Input/PlayerControllerStates.h"

#include "GameData/Input/InputBindings.h"

TEST(InputBindings, PressSingleButtonKeyBinding_NotPressed)
{
	using namespace Input;

	InputBindings bindings;
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Gamepad, 0));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, false);

	EXPECT_EQ(InputBindings::GetKeyState(bindings.keyBindings[GameplayInput::InputKey::Shoot], controllerState), GameplayInput::KeyState::Inactive);
}

TEST(InputBindings, PressSingleButtonKeyBinding_PressKey)
{
	using namespace Input;

	InputBindings bindings;
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Gamepad, 0));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);

	EXPECT_EQ(InputBindings::GetKeyState(bindings.keyBindings[GameplayInput::InputKey::Shoot], controllerState), GameplayInput::MergeKeyStates(GameplayInput::KeyState::Active, GameplayInput::KeyState::JustActivated));
}

TEST(InputBindings, PressSingleButtonKeyBinding_KeepPressing)
{
	using namespace Input;

	InputBindings bindings;
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Gamepad, 0));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState(); // next frame

	// we should update the state for the frame before reading it, so this case doesn't really matter
	// however test it to prevent accidental unwanted changes to the behavior
	EXPECT_EQ(InputBindings::GetKeyState(bindings.keyBindings[GameplayInput::InputKey::Shoot], controllerState), GameplayInput::KeyState::Active);

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);

	EXPECT_EQ(InputBindings::GetKeyState(bindings.keyBindings[GameplayInput::InputKey::Shoot], controllerState), GameplayInput::KeyState::Active);
}

TEST(InputBindings, PressSingleButtonKeyBinding_PressThenRelease)
{
	using namespace Input;

	InputBindings bindings;
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Gamepad, 0));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState(); // next frame
	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, false);

	EXPECT_EQ(InputBindings::GetKeyState(bindings.keyBindings[GameplayInput::InputKey::Shoot], controllerState), GameplayInput::KeyState::JustDeactivated);
}

TEST(InputBindings, PressSingleButtonKeyBinding_PressThenReleaseThenKeepReleasing)
{
	using namespace Input;

	InputBindings bindings;
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Gamepad, 0));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState(); // next frame
	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, false);
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState(); // next frame

	// we should update the state for the frame before reading it, so this case doesn't really matter
	// however test it to prevent accidental unwanted changes to the behavior
	EXPECT_EQ(InputBindings::GetKeyState(bindings.keyBindings[GameplayInput::InputKey::Shoot], controllerState), GameplayInput::KeyState::Inactive);

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, false);

	EXPECT_EQ(InputBindings::GetKeyState(bindings.keyBindings[GameplayInput::InputKey::Shoot], controllerState), GameplayInput::KeyState::Inactive);
}

// some platforms can emulate button clicks by sending press and release events at the same time
TEST(InputBindings, PressSingleButtonKeyBinding_PressAndReleaseSameFrame)
{
	using namespace Input;

	InputBindings bindings;
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Gamepad, 0));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, false);

	EXPECT_EQ(InputBindings::GetKeyState(bindings.keyBindings[GameplayInput::InputKey::Shoot], controllerState), GameplayInput::MergeKeyStates(GameplayInput::KeyState::JustActivated, GameplayInput::KeyState::JustDeactivated));
}

TEST(InputBindings, PressSingleButtonKeyBinding_ReleaseAndPressSameFrame)
{
	using namespace Input;

	InputBindings bindings;
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Gamepad, 0));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState(); // next frame
	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, false);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);

	EXPECT_EQ(InputBindings::GetKeyState(bindings.keyBindings[GameplayInput::InputKey::Shoot], controllerState), GameplayInput::MergeKeyStates(GameplayInput::KeyState::Active, GameplayInput::KeyState::JustDeactivated, GameplayInput::KeyState::JustActivated));
}

TEST(InputBindings, PressSingleButtonKeyBinding_TwoBinds_OnePressed)
{
	using namespace Input;

	InputBindings bindings;
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Gamepad, 0));
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Gamepad, 1));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, false);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, true);

	EXPECT_EQ(InputBindings::GetKeyState(bindings.keyBindings[GameplayInput::InputKey::Shoot], controllerState), GameplayInput::MergeKeyStates(GameplayInput::KeyState::Active, GameplayInput::KeyState::JustActivated));
}

TEST(InputBindings, PressSingleButtonKeyBinding_TwoBinds_BothPressed)
{
	using namespace Input;

	InputBindings bindings;
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Gamepad, 0));
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Gamepad, 1));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, true);

	EXPECT_EQ(InputBindings::GetKeyState(bindings.keyBindings[GameplayInput::InputKey::Shoot], controllerState), GameplayInput::MergeKeyStates(GameplayInput::KeyState::Active, GameplayInput::KeyState::JustActivated));
}

TEST(InputBindings, PressSingleButtonKeyBinding_TwoBinds_BothPressedThenOneReleased)
{
	using namespace Input;

	InputBindings bindings;
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Gamepad, 0));
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Gamepad, 1));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, true);
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState();
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState();
	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, false);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, true);

	EXPECT_EQ(InputBindings::GetKeyState(bindings.keyBindings[GameplayInput::InputKey::Shoot], controllerState), GameplayInput::KeyState::Active);
}

TEST(InputBindings, PressSingleButtonKeyBinding_TwoBinds_OnePressedThenAnother)
{
	using namespace Input;

	InputBindings bindings;
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Gamepad, 0));
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Gamepad, 1));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, false);
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState();
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState();
	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, true);

	EXPECT_EQ(InputBindings::GetKeyState(bindings.keyBindings[GameplayInput::InputKey::Shoot], controllerState), GameplayInput::KeyState::Active);
}

TEST(InputBindings, PressSingleButtonKeyBinding_TwoBinds_BothPressedThenBothReleased)
{
	using namespace Input;

	InputBindings bindings;
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Gamepad, 0));
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Gamepad, 1));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, true);
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState();
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState();
	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, false);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, false);

	EXPECT_EQ(InputBindings::GetKeyState(bindings.keyBindings[GameplayInput::InputKey::Shoot], controllerState), GameplayInput::KeyState::JustDeactivated);
}

TEST(InputBindings, PressSingleButtonKeyBinding_TwoBinds_OnePressedThenReleasedWithOtherPressed)
{
	using namespace Input;

	InputBindings bindings;
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Gamepad, 0));
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Gamepad, 1));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, false);
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState();
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState();
	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, false);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, true);

	EXPECT_EQ(InputBindings::GetKeyState(bindings.keyBindings[GameplayInput::InputKey::Shoot], controllerState), GameplayInput::MergeKeyStates(GameplayInput::KeyState::Active, GameplayInput::KeyState::JustActivated, GameplayInput::KeyState::JustDeactivated));
}

TEST(InputBindings, PressButtonChordKeyBinding_OneKeyFromTwoIsPressed)
{
	using namespace Input;

	InputBindings bindings;
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressButtonChordKeyBinding>(ControllerType::Gamepad, std::vector<int>{ 0, 1 }));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, false);

	EXPECT_EQ(InputBindings::GetKeyState(bindings.keyBindings[GameplayInput::InputKey::Shoot], controllerState), GameplayInput::KeyState::Inactive);
}

TEST(InputBindings, PressButtonChordKeyBinding_AllKeysPressed)
{
	using namespace Input;

	InputBindings bindings;
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressButtonChordKeyBinding>(ControllerType::Gamepad, std::vector<int>{ 0, 1 }));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, true);

	EXPECT_EQ(InputBindings::GetKeyState(bindings.keyBindings[GameplayInput::InputKey::Shoot], controllerState), GameplayInput::MergeKeyStates(GameplayInput::KeyState::Active, GameplayInput::KeyState::JustActivated));
}

TEST(InputBindings, PressButtonChordKeyBinding_AllKeysPressedThenOneReleased)
{
	using namespace Input;

	InputBindings bindings;
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressButtonChordKeyBinding>(ControllerType::Gamepad, std::vector<int>{ 0, 1 }));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, true);
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState();
	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, false);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, true);

	EXPECT_EQ(InputBindings::GetKeyState(bindings.keyBindings[GameplayInput::InputKey::Shoot], controllerState), GameplayInput::KeyState::JustDeactivated);
}

TEST(InputBindings, PressButtonChordKeyBinding_AllKeysPressedThenOneReleasedThenPressedAgain)
{
	using namespace Input;

	InputBindings bindings;
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressButtonChordKeyBinding>(ControllerType::Gamepad, std::vector<int>{ 0, 1 }));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, true);
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState();
	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, false);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, true);
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState();
	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, true);

	EXPECT_EQ(InputBindings::GetKeyState(bindings.keyBindings[GameplayInput::InputKey::Shoot], controllerState), GameplayInput::MergeKeyStates(GameplayInput::KeyState::Active, GameplayInput::KeyState::JustActivated));
}

TEST(InputBindings, PressButtonChordKeyBinding_AllExceptOnePressedThenLastPressedWithOtherReleased)
{
	using namespace Input;

	InputBindings bindings;
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressButtonChordKeyBinding>(ControllerType::Gamepad, std::vector<int>{ 0, 1 }));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, false);
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState();
	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, false);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, true);

	EXPECT_EQ(InputBindings::GetKeyState(bindings.keyBindings[GameplayInput::InputKey::Shoot], controllerState), GameplayInput::KeyState::Inactive);
}

// some platforms can emulate button clicks by sending press and release events at the same time
TEST(InputBindings, PressButtonChordKeyBinding_AllExceptOnePressedThenPressAndReleaseLastButtonSameFrame)
{
	using namespace Input;

	InputBindings bindings;
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressButtonChordKeyBinding>(ControllerType::Gamepad, std::vector<int>{ 0, 1 }));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, false);
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState();
	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, true);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, false);

	EXPECT_EQ(InputBindings::GetKeyState(bindings.keyBindings[GameplayInput::InputKey::Shoot], controllerState), GameplayInput::MergeKeyStates(GameplayInput::KeyState::JustActivated, GameplayInput::KeyState::JustDeactivated));
}

TEST(InputBindings, PressButtonChordKeyBinding_AllPressedThenReleaseAndPressOneButtonSameFrame)
{
	using namespace Input;

	InputBindings bindings;
	bindings.keyBindings[GameplayInput::InputKey::Shoot].emplace_back(std::make_unique<PressButtonChordKeyBinding>(ControllerType::Gamepad, std::vector<int>{ 0, 1 }));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, true);
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState();
	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, false);
	controllerState.getState(ControllerType::Gamepad).updateButtonState(1, true);

	EXPECT_EQ(InputBindings::GetKeyState(bindings.keyBindings[GameplayInput::InputKey::Shoot], controllerState), GameplayInput::MergeKeyStates(GameplayInput::KeyState::Active, GameplayInput::KeyState::JustActivated, GameplayInput::KeyState::JustDeactivated));
}

TEST(InputBindings, PositiveButtonAxisBinding_NotPressed)
{
	using namespace Input;

	InputBindings bindings;
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<PositiveButtonAxisBinding>(ControllerType::Gamepad, 0));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, false);

	EXPECT_FLOAT_EQ(InputBindings::GetBlendedAxisValue(bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal], controllerState), 0.0f);
}

TEST(InputBindings, PositiveButtonAxisBinding_JustPressed)
{
	using namespace Input;

	InputBindings bindings;
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<PositiveButtonAxisBinding>(ControllerType::Gamepad, 0));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);

	EXPECT_FLOAT_EQ(InputBindings::GetBlendedAxisValue(bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal], controllerState), 1.0f);
}

TEST(InputBindings, PositiveButtonAxisBinding_PressedAndHeld)
{
	using namespace Input;

	InputBindings bindings;
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<PositiveButtonAxisBinding>(ControllerType::Gamepad, 0));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState();
	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);

	EXPECT_FLOAT_EQ(InputBindings::GetBlendedAxisValue(bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal], controllerState), 1.0f);
}

TEST(InputBindings, PositiveButtonAxisBinding_PressedAndReleased)
{
	using namespace Input;

	InputBindings bindings;
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<PositiveButtonAxisBinding>(ControllerType::Gamepad, 0));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState();
	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, false);

	EXPECT_FLOAT_EQ(InputBindings::GetBlendedAxisValue(bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal], controllerState), 0.0f);
}

TEST(InputBindings, NegativeButtonAxisBinding_NotPressed)
{
	using namespace Input;

	InputBindings bindings;
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<NegativeButtonAxisBinding>(ControllerType::Gamepad, 0));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, false);

	EXPECT_FLOAT_EQ(InputBindings::GetBlendedAxisValue(bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal], controllerState), 0.0f);
}

TEST(InputBindings, NegativeButtonAxisBinding_JustPressed)
{
	using namespace Input;

	InputBindings bindings;
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<NegativeButtonAxisBinding>(ControllerType::Gamepad, 0));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);

	EXPECT_FLOAT_EQ(InputBindings::GetBlendedAxisValue(bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal], controllerState), -1.0f);
}

TEST(InputBindings, NegativeButtonAxisBinding_PressedAndHeld)
{
	using namespace Input;

	InputBindings bindings;
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<NegativeButtonAxisBinding>(ControllerType::Gamepad, 0));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState();
	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);

	EXPECT_FLOAT_EQ(InputBindings::GetBlendedAxisValue(bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal], controllerState), -1.0f);
}

TEST(InputBindings, NegativeButtonAxisBinding_PressedAndReleased)
{
	using namespace Input;

	InputBindings bindings;
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<NegativeButtonAxisBinding>(ControllerType::Gamepad, 0));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, true);
	controllerState.getState(ControllerType::Gamepad).clearLastFrameState();
	controllerState.getState(ControllerType::Gamepad).updateButtonState(0, false);

	EXPECT_FLOAT_EQ(InputBindings::GetBlendedAxisValue(bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal], controllerState), 0.0f);
}

TEST(InputBindings, DirectAxisToAxisBinding_OneBinding)
{
	using namespace Input;

	InputBindings bindings;
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<DirectAxisToAxisBinding>(ControllerType::Gamepad, 0));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateAxis(0, 0.0f);
	EXPECT_FLOAT_EQ(InputBindings::GetBlendedAxisValue(bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal], controllerState), 0.0f);
	controllerState.getState(ControllerType::Gamepad).updateAxis(0, 1.0f);
	EXPECT_FLOAT_EQ(InputBindings::GetBlendedAxisValue(bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal], controllerState), 1.0f);
	controllerState.getState(ControllerType::Gamepad).updateAxis(0, 0.5f);
	EXPECT_FLOAT_EQ(InputBindings::GetBlendedAxisValue(bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal], controllerState), 0.5f);
	controllerState.getState(ControllerType::Gamepad).updateAxis(0, -0.5f);
	EXPECT_FLOAT_EQ(InputBindings::GetBlendedAxisValue(bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal], controllerState), -0.5f);
	controllerState.getState(ControllerType::Gamepad).updateAxis(0, -386.0f);
	EXPECT_FLOAT_EQ(InputBindings::GetBlendedAxisValue(bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal], controllerState), -386.0f);
}

TEST(InputBindings, DirectAxisToAxisBinding_TwoBindings_BothZero)
{
	using namespace Input;

	InputBindings bindings;
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<DirectAxisToAxisBinding>(ControllerType::Gamepad, 0));
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<DirectAxisToAxisBinding>(ControllerType::Gamepad, 1));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateAxis(0, 0.0f);
	controllerState.getState(ControllerType::Gamepad).updateAxis(1, 0.0f);
	EXPECT_FLOAT_EQ(InputBindings::GetBlendedAxisValue(bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal], controllerState), 0.0f);
}

TEST(InputBindings, DirectAxisToAxisBinding_TwoBindings_BothOne)
{
	using namespace Input;

	InputBindings bindings;
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<DirectAxisToAxisBinding>(ControllerType::Gamepad, 0));
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<DirectAxisToAxisBinding>(ControllerType::Gamepad, 1));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateAxis(0, 1.0f);
	controllerState.getState(ControllerType::Gamepad).updateAxis(1, 1.0f);
	EXPECT_FLOAT_EQ(InputBindings::GetBlendedAxisValue(bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal], controllerState), 1.0f);
}

TEST(InputBindings, DirectAxisToAxisBinding_TwoBindings_BothHalf)
{
	using namespace Input;

	InputBindings bindings;
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<DirectAxisToAxisBinding>(ControllerType::Gamepad, 0));
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<DirectAxisToAxisBinding>(ControllerType::Gamepad, 1));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateAxis(0, 0.5f);
	controllerState.getState(ControllerType::Gamepad).updateAxis(1, 0.5f);
	EXPECT_FLOAT_EQ(InputBindings::GetBlendedAxisValue(bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal], controllerState), 0.5f);
}

TEST(InputBindings, DirectAxisToAxisBinding_TwoBindings_FullOpposite)
{
	using namespace Input;

	InputBindings bindings;
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<DirectAxisToAxisBinding>(ControllerType::Gamepad, 0));
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<DirectAxisToAxisBinding>(ControllerType::Gamepad, 1));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateAxis(0, -1.0f);
	controllerState.getState(ControllerType::Gamepad).updateAxis(1, 1.0f);
	EXPECT_FLOAT_EQ(InputBindings::GetBlendedAxisValue(bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal], controllerState), 0.0f);
}

TEST(InputBindings, DirectAxisToAxisBinding_TwoBindings_OneAndZero)
{
	using namespace Input;

	InputBindings bindings;
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<DirectAxisToAxisBinding>(ControllerType::Gamepad, 0));
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<DirectAxisToAxisBinding>(ControllerType::Gamepad, 1));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateAxis(0, 1.0f);
	controllerState.getState(ControllerType::Gamepad).updateAxis(1, 0.0f);
	EXPECT_FLOAT_EQ(InputBindings::GetBlendedAxisValue(bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal], controllerState), 1.0f);
}

TEST(InputBindings, DirectAxisToAxisBinding_TwoBindings_MinusOneAndZero)
{
	using namespace Input;

	InputBindings bindings;
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<DirectAxisToAxisBinding>(ControllerType::Gamepad, 0));
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<DirectAxisToAxisBinding>(ControllerType::Gamepad, 1));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateAxis(0, -1.0f);
	controllerState.getState(ControllerType::Gamepad).updateAxis(1, 0.0f);
	EXPECT_FLOAT_EQ(InputBindings::GetBlendedAxisValue(bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal], controllerState), -1.0f);
}

TEST(InputBindings, DirectAxisToAxisBinding_TwoBindings_OneAndHalf)
{
	using namespace Input;

	InputBindings bindings;
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<DirectAxisToAxisBinding>(ControllerType::Gamepad, 0));
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<DirectAxisToAxisBinding>(ControllerType::Gamepad, 1));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateAxis(0, 1.0f);
	controllerState.getState(ControllerType::Gamepad).updateAxis(1, 0.5f);
	EXPECT_FLOAT_EQ(InputBindings::GetBlendedAxisValue(bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal], controllerState), 1.0f);
}

TEST(InputBindings, DirectAxisToAxisBinding_TwoBindings_FullOneDirrectionAndBitOpposite)
{
	using namespace Input;

	InputBindings bindings;
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<DirectAxisToAxisBinding>(ControllerType::Gamepad, 0));
	bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal].emplace_back(std::make_unique<DirectAxisToAxisBinding>(ControllerType::Gamepad, 1));
	PlayerControllerStates controllerState;

	controllerState.getState(ControllerType::Gamepad).updateAxis(0, 1.0f);
	controllerState.getState(ControllerType::Gamepad).updateAxis(1, -0.1f);

	const float resultAxisValue = InputBindings::GetBlendedAxisValue(bindings.axisBindings[GameplayInput::InputAxis::MoveHorizontal], controllerState);

	// we shouldn't have full positive result if we have a binding that adds a negative value
	EXPECT_LT(resultAxisValue, 1.0f);
	// 0.5f is too low result for just 10% opposite value, this can cause abrupt speed change when slightly changing axis position
	EXPECT_GT(resultAxisValue, 0.5f);

	// the exact value is not important enough, this check is just to catch unexpected changes in the logic
	EXPECT_FLOAT_EQ(resultAxisValue, 0.9f);
}

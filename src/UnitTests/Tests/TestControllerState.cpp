#include "Base/precomp.h"

#include <gtest/gtest.h>

#include "GameData/Input/ControllerState.h"

TEST(ControllerState, ButtonNotPressed)
{
	Input::ControllerState state;
	state.updateButtonState(0, false);

	EXPECT_FALSE(state.isButtonJustPressed(0));
	EXPECT_FALSE(state.isButtonPressed(0));
	EXPECT_FALSE(state.isButtonJustReleased(0));
}

TEST(ControllerState, ButtonPressedThisFrame)
{
	Input::ControllerState state;
	state.updateButtonState(0, true);

	EXPECT_TRUE(state.isButtonJustPressed(0));
	EXPECT_TRUE(state.isButtonPressed(0));
	EXPECT_FALSE(state.isButtonJustReleased(0));
}

TEST(ControllerState, ButtonPressedAndHeld)
{
	Input::ControllerState state;
	state.updateButtonState(0, true);
	state.clearLastFrameState();

	EXPECT_FALSE(state.isButtonJustPressed(0));
	EXPECT_TRUE(state.isButtonPressed(0));
	EXPECT_FALSE(state.isButtonJustReleased(0));
}

TEST(ControllerState, ButtonPressedAndThenReleased)
{
	Input::ControllerState state;
	state.updateButtonState(0, true);
	state.clearLastFrameState();
	state.updateButtonState(0, false);

	EXPECT_FALSE(state.isButtonJustPressed(0));
	EXPECT_FALSE(state.isButtonPressed(0));
	EXPECT_TRUE(state.isButtonJustReleased(0));
}

// some platforms can emulate button clicks by sending press and release events at the same time
TEST(ControllerState, ButtonPressedAndReleasedWithinSameFrame)
{
	Input::ControllerState state;
	state.updateButtonState(0, true);
	state.updateButtonState(0, false);

	EXPECT_TRUE(state.isButtonJustPressed(0));
	EXPECT_FALSE(state.isButtonPressed(0));
	EXPECT_TRUE(state.isButtonJustReleased(0));
}

TEST(ControllerState, ButtonReleasedAndPressedWithinSameFrame)
{
	Input::ControllerState state;
	state.updateButtonState(0, true);
	state.clearLastFrameState();
	state.updateButtonState(0, false);
	state.updateButtonState(0, true);

	EXPECT_TRUE(state.isButtonJustPressed(0));
	EXPECT_TRUE(state.isButtonPressed(0));
	EXPECT_TRUE(state.isButtonJustReleased(0));
}

TEST(ControllerState, UpdateAxis)
{
	Input::ControllerState state;
	state.updateAxis(0, 0.5f);

	EXPECT_EQ(state.getAxisValue(0), 0.5f);

	state.clearLastFrameState();

	EXPECT_EQ(state.getAxisValue(0), 0.5f);
}

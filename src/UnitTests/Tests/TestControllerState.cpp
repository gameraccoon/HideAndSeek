#include "EngineCommon/precomp.h"

#include <gtest/gtest.h>

#include "EngineData/Input/ControllerState.h"

namespace ControllerStateTestsInternal
{
	using ControllerState = Input::ControllerState<1, 1>;
}

TEST(ControllerState, ButtonNotPressed)
{
	ControllerStateTestsInternal::ControllerState state;
	state.updateButtonState(0, false);

	EXPECT_FALSE(state.isButtonJustPressed(0));
	EXPECT_FALSE(state.isButtonPressed(0));
	EXPECT_FALSE(state.isButtonJustReleased(0));
}

TEST(ControllerState, ButtonPressedThisFrame)
{
	ControllerStateTestsInternal::ControllerState state;
	state.updateButtonState(0, true);

	EXPECT_TRUE(state.isButtonJustPressed(0));
	EXPECT_TRUE(state.isButtonPressed(0));
	EXPECT_FALSE(state.isButtonJustReleased(0));
}

TEST(ControllerState, ButtonPressedAndHeld)
{
	ControllerStateTestsInternal::ControllerState state;
	state.updateButtonState(0, true);
	state.clearLastFrameState();

	EXPECT_FALSE(state.isButtonJustPressed(0));
	EXPECT_TRUE(state.isButtonPressed(0));
	EXPECT_FALSE(state.isButtonJustReleased(0));
}

TEST(ControllerState, ButtonPressedAndThenReleased)
{
	ControllerStateTestsInternal::ControllerState state;
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
	ControllerStateTestsInternal::ControllerState state;
	state.updateButtonState(0, true);
	state.updateButtonState(0, false);

	EXPECT_TRUE(state.isButtonJustPressed(0));
	EXPECT_FALSE(state.isButtonPressed(0));
	EXPECT_TRUE(state.isButtonJustReleased(0));
}

TEST(ControllerState, ButtonReleasedAndPressedWithinSameFrame)
{
	ControllerStateTestsInternal::ControllerState state;
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
	ControllerStateTestsInternal::ControllerState state;
	state.updateAxis(0, 0.5f);

	EXPECT_EQ(state.getAxisValue(0), 0.5f);

	state.clearLastFrameState();

	EXPECT_EQ(state.getAxisValue(0), 0.5f);
}

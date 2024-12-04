#include "EngineCommon/precomp.h"

#include <gtest/gtest.h>

#include "GameData/Input/GameplayInputFrameState.h"

TEST(GameplayInput, KeyInactive)
{
	GameplayInput::FrameState state;
	state.updateKey(GameplayInput::InputKey::Shoot, GameplayInput::KeyState::Inactive, GameplayTimestamp(1));

	EXPECT_EQ(state.getKeyState(GameplayInput::InputKey::Shoot), GameplayInput::KeyState::Inactive);
	EXPECT_FALSE(state.isKeyJustActivated(GameplayInput::InputKey::Shoot));
	EXPECT_FALSE(state.isKeyActive(GameplayInput::InputKey::Shoot));
	EXPECT_FALSE(state.isKeyJustDeactivated(GameplayInput::InputKey::Shoot));
	EXPECT_EQ(state.getLastFlipTime(GameplayInput::InputKey::Shoot), GameplayTimestamp(1));
}

TEST(GameplayInput, KeyJustActivated)
{
	GameplayInput::FrameState state;
	state.updateKey(GameplayInput::InputKey::Shoot, MergeKeyStates(GameplayInput::KeyState::Active, GameplayInput::KeyState::JustActivated), GameplayTimestamp(1));

	EXPECT_EQ(state.getKeyState(GameplayInput::InputKey::Shoot), MergeKeyStates(GameplayInput::KeyState::Active, GameplayInput::KeyState::JustActivated));
	EXPECT_TRUE(state.isKeyJustActivated(GameplayInput::InputKey::Shoot));
	EXPECT_TRUE(state.isKeyActive(GameplayInput::InputKey::Shoot));
	EXPECT_FALSE(state.isKeyJustDeactivated(GameplayInput::InputKey::Shoot));
	EXPECT_EQ(state.getLastFlipTime(GameplayInput::InputKey::Shoot), GameplayTimestamp(1));
}

TEST(GameplayInput, KeyActive)
{
	GameplayInput::FrameState state;
	state.updateKey(GameplayInput::InputKey::Shoot, GameplayInput::KeyState::Active, GameplayTimestamp(1));

	EXPECT_EQ(state.getKeyState(GameplayInput::InputKey::Shoot), GameplayInput::KeyState::Active);
	EXPECT_FALSE(state.isKeyJustActivated(GameplayInput::InputKey::Shoot));
	EXPECT_TRUE(state.isKeyActive(GameplayInput::InputKey::Shoot));
	EXPECT_FALSE(state.isKeyJustDeactivated(GameplayInput::InputKey::Shoot));
	EXPECT_EQ(state.getLastFlipTime(GameplayInput::InputKey::Shoot), GameplayTimestamp(1));
}

TEST(GameplayInput, KeyJustDeactivated)
{
	GameplayInput::FrameState state;
	state.updateKey(GameplayInput::InputKey::Shoot, GameplayInput::KeyState::JustDeactivated, GameplayTimestamp(1));

	EXPECT_EQ(state.getKeyState(GameplayInput::InputKey::Shoot), GameplayInput::KeyState::JustDeactivated);
	EXPECT_FALSE(state.isKeyJustActivated(GameplayInput::InputKey::Shoot));
	EXPECT_FALSE(state.isKeyActive(GameplayInput::InputKey::Shoot));
	EXPECT_TRUE(state.isKeyJustDeactivated(GameplayInput::InputKey::Shoot));
	EXPECT_EQ(state.getLastFlipTime(GameplayInput::InputKey::Shoot), GameplayTimestamp(1));
}

TEST(GameplayInput, KeyActivationTime)
{
	GameplayInput::FrameState state;
	// before we ever updated any state the timestamp is uninitialized
	EXPECT_FALSE(state.getLastFlipTime(GameplayInput::InputKey::Shoot).isInitialized());

	state.updateKey(GameplayInput::InputKey::Shoot, GameplayInput::KeyState::Inactive, GameplayTimestamp(1));
	// first update always updates the timestamp to have some valid time
	EXPECT_EQ(state.getLastFlipTime(GameplayInput::InputKey::Shoot), GameplayTimestamp(1));

	state.updateKey(GameplayInput::InputKey::Shoot, GameplayInput::KeyState::Inactive, GameplayTimestamp(2));
	EXPECT_EQ(state.getLastFlipTime(GameplayInput::InputKey::Shoot), GameplayTimestamp(1));

	state.updateKey(GameplayInput::InputKey::Shoot, MergeKeyStates(GameplayInput::KeyState::Active, GameplayInput::KeyState::JustActivated), GameplayTimestamp(3));
	EXPECT_EQ(state.getLastFlipTime(GameplayInput::InputKey::Shoot), GameplayTimestamp(3));

	state.updateKey(GameplayInput::InputKey::Shoot, GameplayInput::KeyState::Active, GameplayTimestamp(4));
	EXPECT_EQ(state.getLastFlipTime(GameplayInput::InputKey::Shoot), GameplayTimestamp(3));

	state.updateKey(GameplayInput::InputKey::Shoot, GameplayInput::KeyState::Active, GameplayTimestamp(5));
	EXPECT_EQ(state.getLastFlipTime(GameplayInput::InputKey::Shoot), GameplayTimestamp(3));

	state.updateKey(GameplayInput::InputKey::Shoot, GameplayInput::KeyState::JustDeactivated, GameplayTimestamp(6));
	EXPECT_EQ(state.getLastFlipTime(GameplayInput::InputKey::Shoot), GameplayTimestamp(6));

	state.updateKey(GameplayInput::InputKey::Shoot, GameplayInput::KeyState::Inactive, GameplayTimestamp(7));
	EXPECT_EQ(state.getLastFlipTime(GameplayInput::InputKey::Shoot), GameplayTimestamp(6));
}

TEST(GameplayInput, UpdateAxis)
{
	GameplayInput::FrameState state;
	state.updateAxis(GameplayInput::InputAxis::AimHorizontal, 0.5f);
	EXPECT_EQ(state.getAxisValue(GameplayInput::InputAxis::AimHorizontal), 0.5f);
	state.updateAxis(GameplayInput::InputAxis::AimHorizontal, -0.25f);
	EXPECT_EQ(state.getAxisValue(GameplayInput::InputAxis::AimHorizontal), -0.25f);
}

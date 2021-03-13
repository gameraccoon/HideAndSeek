#include "Base/precomp.h"

#include <gtest/gtest.h>

#include "GameData/FSM/StateMachine.h"

enum class TestStates
{
	StateOne,
	StateTwo,
	StateThree,
	StateFour,
	ParentStateOne,
	ParentStateTwo
};

enum class BlackboardTestValues
{
	One,
	Two,
	Three
};

TEST(FSM, Blackboard)
{
	{
		FSM::Blackboard<std::string> blackboard;

		blackboard.setValue<int>("test", 1);

		EXPECT_EQ(1, blackboard.getValue<int>("test"));
		EXPECT_EQ(0, blackboard.getValue<int>("non-existent"));
		EXPECT_EQ(3, blackboard.getValue<int>("non-existent", 3));
	}

	{
		FSM::Blackboard<BlackboardTestValues> blackboard;

		blackboard.setValue<int>(BlackboardTestValues::One, 1);

		EXPECT_EQ(1, blackboard.getValue<int>(BlackboardTestValues::One));
		EXPECT_EQ(0, blackboard.getValue<int>(BlackboardTestValues::Two));
		EXPECT_EQ(3, blackboard.getValue<int>(BlackboardTestValues::Two, 3));
	}
}

TEST(FSM, BasicStateAndLink)
{
	using StateIDType = TestStates;
	using BlackBoardKeyType = BlackboardTestValues;
	using TestFSM = FSM::StateMachine<StateIDType, BlackBoardKeyType>;

	FSM::Blackboard<BlackBoardKeyType> blackboard;
	TestFSM fsm;
	StateIDType currentState = TestStates::StateOne;

	TestFSM::StateLinkRules stateOneLinks;
	stateOneLinks.emplaceLink<FSM::LinkRules::FunctorLink>(TestStates::StateTwo, [](const TestFSM::BlackboardType&)
	{
		return true;
	});
	fsm.addState(TestStates::StateOne, std::move(stateOneLinks));

	fsm.addState(TestStates::StateTwo, TestFSM::StateLinkRules());

	currentState = fsm.getNextState(blackboard, currentState);

	EXPECT_EQ(TestStates::StateTwo, currentState);
}

TEST(FSM, EqualityLinkRule)
{
	using StateIDType = TestStates;
	using BlackBoardKeyType = BlackboardTestValues;
	using TestFSM = FSM::StateMachine<StateIDType, BlackBoardKeyType>;

	FSM::Blackboard<BlackBoardKeyType> blackboard;
	TestFSM fsm;
	StateIDType currentState = TestStates::StateOne;

	TestFSM::StateLinkRules stateOneLinks;
	stateOneLinks.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(TestStates::StateTwo, BlackboardTestValues::One, true);
	fsm.addState(TestStates::StateOne, std::move(stateOneLinks));

	TestFSM::StateLinkRules stateTwoLinks;
	stateTwoLinks.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(TestStates::StateOne, BlackboardTestValues::One, false);
	fsm.addState(TestStates::StateTwo, std::move(stateTwoLinks));

	currentState = fsm.getNextState(blackboard, currentState);

	EXPECT_EQ(TestStates::StateOne, currentState);

	blackboard.setValue<bool>(BlackboardTestValues::One, true);
	currentState = fsm.getNextState(blackboard, currentState);

	EXPECT_EQ(TestStates::StateTwo, currentState);

	blackboard.setValue<bool>(BlackboardTestValues::One, false);
	currentState = fsm.getNextState(blackboard, currentState);

	EXPECT_EQ(TestStates::StateOne, currentState);
}

TEST(FSM, DoubleLinkJump)
{
	using StateIDType = TestStates;
	using BlackBoardKeyType = std::string;
	using TestFSM = FSM::StateMachine<StateIDType, BlackBoardKeyType>;

	FSM::Blackboard<BlackBoardKeyType> blackboard;
	TestFSM fsm;
	StateIDType currentState = TestStates::StateOne;

	TestFSM::StateLinkRules stateOneLinks;
	stateOneLinks.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(TestStates::StateTwo, "stepTwo", true);
	fsm.addState(TestStates::StateOne, std::move(stateOneLinks));

	TestFSM::StateLinkRules stateTwoLinks;
	stateTwoLinks.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(TestStates::StateThree, "stepOne", true);
	fsm.addState(TestStates::StateTwo, std::move(stateTwoLinks));

	fsm.addState(TestStates::StateThree, TestFSM::StateLinkRules());

	currentState = fsm.getNextState(blackboard, currentState);

	EXPECT_EQ(TestStates::StateOne, currentState);

	blackboard.setValue<bool>("stepOne", true);
	currentState = fsm.getNextState(blackboard, currentState);

	EXPECT_EQ(TestStates::StateOne, currentState);

	blackboard.setValue<bool>("stepTwo", true);
	currentState = fsm.getNextState(blackboard, currentState);

	EXPECT_EQ(TestStates::StateThree, currentState);
}

TEST(FSM, HierarchicalStateMachine)
{
	using StateIDType = TestStates;
	using BlackBoardKeyType = std::string;
	using TestFSM = FSM::StateMachine<StateIDType, BlackBoardKeyType>;

	FSM::Blackboard<BlackBoardKeyType> blackboard;
	TestFSM fsm;
	StateIDType currentState = TestStates::StateOne;

	TestFSM::StateLinkRules stateOneLinks;
	stateOneLinks.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(TestStates::StateTwo, "stepOne", true);
	fsm.addState(TestStates::StateOne, std::move(stateOneLinks));

	TestFSM::StateLinkRules stateTwoLinks;
	stateTwoLinks.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(TestStates::StateThree, "stepTwo", true);
	fsm.addState(TestStates::StateTwo, std::move(stateTwoLinks));

	fsm.addState(TestStates::StateThree, TestFSM::StateLinkRules());

	TestFSM::StateLinkRules parentStateOneLinks;
	parentStateOneLinks.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(TestStates::StateFour, "stepTwo", true);
	fsm.addState(TestStates::ParentStateOne, std::move(parentStateOneLinks));
	fsm.linkStates(TestStates::StateOne, TestStates::ParentStateOne, true);
	fsm.linkStates(TestStates::StateTwo, TestStates::ParentStateOne);
	fsm.linkStates(TestStates::StateThree, TestStates::ParentStateOne);

	TestFSM::StateLinkRules stateFourLinks;
	stateFourLinks.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(TestStates::ParentStateOne, "stepThree", true);
	fsm.addState(TestStates::StateFour, std::move(stateFourLinks));

	currentState = fsm.getNextState(blackboard, currentState);

	EXPECT_EQ(TestStates::StateOne, currentState);

	blackboard.setValue<bool>("stepOne", true);
	currentState = fsm.getNextState(blackboard, currentState);

	EXPECT_EQ(TestStates::StateTwo, currentState);

	blackboard.setValue<bool>("stepTwo", true);
	currentState = fsm.getNextState(blackboard, currentState);

	EXPECT_EQ(TestStates::StateFour, currentState);

	blackboard.setValue<bool>("stepTwo", false);
	blackboard.setValue<bool>("stepThree", true);
	currentState = fsm.getNextState(blackboard, currentState);

	EXPECT_EQ(TestStates::StateTwo, currentState);
}

TEST(FSM, HierarchicalFSMStringKeys_BugfixTest)
{
	using StateIDType = std::string;
	using BlackBoardKeyType = std::string;
	using TestFSM = FSM::StateMachine<StateIDType, BlackBoardKeyType>;

	TestFSM fsm;

	// Top level states
	{
		TestFSM::StateLinkRules rules;
		rules.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>("idle", "TryingToMove", false);
		rules.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>("run", "ReadyToRun", true);
		fsm.addState("metawalk", std::move(rules));
	}
	{
		TestFSM::StateLinkRules rules;
		rules.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>("metawalk", "TryingToMove", true);
		fsm.addState("idle", std::move(rules));
	}
	// Substates of MetaWalk
	{
		TestFSM::StateLinkRules rules;
		rules.emplaceLink<FSM::LinkRules::VariableEqualLink, int>("strafeleft", "Direction", 1);
		rules.emplaceLink<FSM::LinkRules::VariableEqualLink, int>("straferight", "Direction", 2);
		fsm.addState("walk", std::move(rules));
		fsm.linkStates("walk", "metawalk", true);
	}
	{
		TestFSM::StateLinkRules rules;
		rules.emplaceLink<FSM::LinkRules::VariableEqualLink, int>("walk", "Direction", 3);
		rules.emplaceLink<FSM::LinkRules::VariableEqualLink, int>("walk", "Direction", 4);
		fsm.addState("strafeleft", std::move(rules));
		fsm.linkStates("strafeleft", "metawalk", false);
	}
	{
		TestFSM::StateLinkRules rules;
		rules.emplaceLink<FSM::LinkRules::VariableEqualLink, int>("walk", "Direction", 3);
		rules.emplaceLink<FSM::LinkRules::VariableEqualLink, int>("walk", "Direction", 4);
		fsm.addState("straferight", std::move(rules));
		fsm.linkStates("straferight", "metawalk", false);
	}

	FSM::Blackboard<BlackBoardKeyType> blackboard;
	blackboard.setValue<bool>("ReadyToRun", false);
	blackboard.setValue<bool>("TryingToMove", true);
	blackboard.setValue<int>("Direction", 2);

	EXPECT_EQ("straferight", fsm.getNextState(blackboard, "idle"));
}

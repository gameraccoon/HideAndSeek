#include "EngineCommon/precomp.h"

#include "GameLogic/Initialization/StateMachines.h"

#include "GameData/Enums/MoveDirection4.generated.h"

namespace StateMachines
{
	static void RegisterCharacterSM(FSM::StateMachine<CharacterState, CharacterStateBlackboardKeys>& sm)
	{
		using FSMType = FSM::StateMachine<CharacterState, CharacterStateBlackboardKeys>;
		{
			FSMType::StateLinkRules rules;
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(CharacterState::Walk, CharacterStateBlackboardKeys::TryingToMove, true);
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(CharacterState::Shoot, CharacterStateBlackboardKeys::TryingToShoot, true);
			sm.addState(CharacterState::Idle, std::move(rules));
		}
		{
			FSMType::StateLinkRules rules;
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(CharacterState::Idle, CharacterStateBlackboardKeys::TryingToMove, false);
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(CharacterState::WalkAndShoot, CharacterStateBlackboardKeys::TryingToShoot, true);
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(CharacterState::Run, CharacterStateBlackboardKeys::ReadyToRun, true);
			sm.addState(CharacterState::Walk, std::move(rules));
		}
		{
			FSMType::StateLinkRules rules;
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(CharacterState::WalkAndShoot, CharacterStateBlackboardKeys::TryingToMove, true);
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(CharacterState::Idle, CharacterStateBlackboardKeys::TryingToShoot, false);
			sm.addState(CharacterState::Shoot, std::move(rules));
		}
		{
			FSMType::StateLinkRules rules;
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(CharacterState::Shoot, CharacterStateBlackboardKeys::TryingToMove, false);
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(CharacterState::Walk, CharacterStateBlackboardKeys::TryingToShoot, false);
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(CharacterState::Run, CharacterStateBlackboardKeys::ReadyToRun, true);
			sm.addState(CharacterState::WalkAndShoot, std::move(rules));
		}
		{
			FSMType::StateLinkRules rules;
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(CharacterState::Walk, CharacterStateBlackboardKeys::ReadyToRun, false);
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(CharacterState::Idle, CharacterStateBlackboardKeys::TryingToMove, false);
			sm.addState(CharacterState::Run, std::move(rules));
		}
	}

	static void RegisterCharacterAnimationSM(FSM::StateMachine<StringId, StringId>& sm)
	{
		using FSMType = FSM::StateMachine<StringId, StringId>;

		{
			FSMType::StateLinkRules rules;
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, StringId>(STR_TO_ID("move"), STR_TO_ID("charState"), enum_to_string(CharacterState::Walk));
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, StringId>(STR_TO_ID("move"), STR_TO_ID("charState"), enum_to_string(CharacterState::Run));
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, StringId>(STR_TO_ID("shoot"), STR_TO_ID("charState"), enum_to_string(CharacterState::Shoot));
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, StringId>(STR_TO_ID("shoot"), STR_TO_ID("charState"), enum_to_string(CharacterState::WalkAndShoot));
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, StringId>(STR_TO_ID("reload"), STR_TO_ID("charState"), enum_to_string(CharacterState::Reload));
			sm.addState(STR_TO_ID("idle"), std::move(rules));
		}
		{
			FSMType::StateLinkRules rules;
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, StringId>(STR_TO_ID("idle"), STR_TO_ID("charState"), enum_to_string(CharacterState::Idle));
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, StringId>(STR_TO_ID("reload"), STR_TO_ID("charState"), enum_to_string(CharacterState::Reload));
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, StringId>(STR_TO_ID("shoot"), STR_TO_ID("charState"), enum_to_string(CharacterState::Shoot));
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, StringId>(STR_TO_ID("shoot"), STR_TO_ID("charState"), enum_to_string(CharacterState::WalkAndShoot));
			sm.addState(STR_TO_ID("move"), std::move(rules));
		}
		{
			FSMType::StateLinkRules rules;
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, StringId>(STR_TO_ID("idle"), STR_TO_ID("charState"), enum_to_string(CharacterState::Idle));
			sm.addState(STR_TO_ID("reload"), std::move(rules));
		}
		{
			FSMType::StateLinkRules rules;
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, StringId>(STR_TO_ID("idle"), STR_TO_ID("charState"), enum_to_string(CharacterState::Idle));
			sm.addState(STR_TO_ID("shoot"), std::move(rules));
		}
	}

	static void RegisterCharacterLegsAnimationSM(FSM::StateMachine<StringId, StringId>& sm)
	{
		using FSMType = FSM::StateMachine<StringId, StringId>;

		// Top level states
		{
			FSMType::StateLinkRules rules;
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(STR_TO_ID("idle"), enum_to_string(CharacterStateBlackboardKeys::TryingToMove), false);
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(STR_TO_ID("run"), enum_to_string(CharacterStateBlackboardKeys::ReadyToRun), true);
			sm.addState(STR_TO_ID("metawalk"), std::move(rules));
		}
		{
			FSMType::StateLinkRules rules;
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(STR_TO_ID("metawalk"), enum_to_string(CharacterStateBlackboardKeys::TryingToMove), true);
			sm.addState(STR_TO_ID("idle"), std::move(rules));
		}
		{
			FSMType::StateLinkRules rules;
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(STR_TO_ID("metawalk"), enum_to_string(CharacterStateBlackboardKeys::ReadyToRun), false);
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, bool>(STR_TO_ID("idle"), enum_to_string(CharacterStateBlackboardKeys::TryingToMove), false);
			sm.addState(STR_TO_ID("run"), std::move(rules));
		}
		// Substates of MetaWalk
		{
			FSMType::StateLinkRules rules;
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, MoveDirection4>(STR_TO_ID("strafeleft"), STR_TO_ID("moveDir"), MoveDirection4::Left);
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, MoveDirection4>(STR_TO_ID("straferight"), STR_TO_ID("moveDir"), MoveDirection4::Right);
			sm.addState(STR_TO_ID("walk"), std::move(rules));
			sm.linkStates(STR_TO_ID("walk"), STR_TO_ID("metawalk"), true);
		}
		{
			FSMType::StateLinkRules rules;
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, MoveDirection4>(STR_TO_ID("walk"), STR_TO_ID("moveDir"), MoveDirection4::Front);
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, MoveDirection4>(STR_TO_ID("walk"), STR_TO_ID("moveDir"), MoveDirection4::Back);
			sm.addState(STR_TO_ID("strafeleft"), std::move(rules));
			sm.linkStates(STR_TO_ID("strafeleft"), STR_TO_ID("metawalk"), false);
		}
		{
			FSMType::StateLinkRules rules;
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, MoveDirection4>(STR_TO_ID("walk"), STR_TO_ID("moveDir"), MoveDirection4::Front);
			rules.emplaceLink<FSM::LinkRules::VariableEqualLink, MoveDirection4>(STR_TO_ID("walk"), STR_TO_ID("moveDir"), MoveDirection4::Back);
			sm.addState(STR_TO_ID("straferight"), std::move(rules));
			sm.linkStates(STR_TO_ID("straferight"), STR_TO_ID("metawalk"), false);
		}
	}

	void RegisterStateMachines(StateMachineComponent *stateMachine)
	{
		if (!stateMachine)
		{
			return;
		}

		RegisterCharacterSM(stateMachine->getCharacterSMRef());
		RegisterCharacterAnimationSM(stateMachine->getAnimationSMsRef()[STR_TO_ID("handgun")]);
		RegisterCharacterLegsAnimationSM(stateMachine->getAnimationSMsRef()[STR_TO_ID("feet")]);
	}
}

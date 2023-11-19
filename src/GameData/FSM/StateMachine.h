#pragma once

#include <vector>
#include <map>
#include <memory>

#include "Base/Debug/Assert.h"
#include "Base/CompilerHelpers.h"

#include "GameData/FSM/Blackboard.h"
#include "GameData/FSM/Link.h"

namespace FSM
{
	/**
	 * Hierarchical FSM implementation
	 *
	 * The SM can be initialized by adding states via calling addState method.
	 * The current state and the blackboard are stored outside the class, allowing
	 * using one SM for multiple state instances.
	 *
	 * Allow to have more than one levels of states.
	 * Each parent node should have an unique state for identification purposes, but selecting a parent
	 * node will result in selecting its default child node (if it was set on initialization).
	 *
	 * The same behavior can be achieved with StateMachine class (and in some cases more efficient),
	 * but it will result in links duplication and will be harder to reason about.
	 */
	template <typename StateIdType, typename BlackboardKeyType>
	class StateMachine
	{
	public:
		using BlackboardType = Blackboard<BlackboardKeyType>;
		using BaseLinkRuleType = LinkRule<BlackboardKeyType>;

		struct LinkPair
		{
			LinkPair(StateIdType followingState, std::unique_ptr<BaseLinkRuleType> linkFollowRule)
				: followingState(std::forward<StateIdType>(followingState))
				, linkFollowRule(std::forward<std::unique_ptr<BaseLinkRuleType>>(linkFollowRule))
			{}

			~LinkPair() = default;
			LinkPair(LinkPair&& other) noexcept = default;
			LinkPair& operator=(LinkPair&& other) noexcept = default;

			LinkPair(const LinkPair& other)
				: LinkPair(other.followingState, other.linkFollowRule->makeCopy())
			{}

			LinkPair& operator=(const LinkPair& other)
			{
				followingState = other.followingState;
				linkFollowRule = other.linkFollowRule->makeCopy();
				return *this;
			}

			StateIdType followingState;
			std::unique_ptr<BaseLinkRuleType> linkFollowRule;
		};

		struct StateLinkRules
		{
			// less verbose emplace function
			template <template<typename...> typename LinkRuleType, typename... Types, typename... Args>
			void emplaceLink(StateIdType state, Args&&... args)
			{
				links.emplace_back(std::move(state), std::make_unique<LinkRuleType<BlackboardKeyType, Types...>>(std::forward<Args>(args)...));
			}

			std::vector<LinkPair> links;
		};

	public:
		void addState(StateIdType stateId, StateLinkRules&& stateLinkRules)
		{
			bool isAdded;
			std::tie(std::ignore, isAdded) = mStates.emplace(std::forward<StateIdType>(stateId), std::forward<StateLinkRules>(stateLinkRules));
			Assert(isAdded, "State is already exists");
		}

		void linkStates(StateIdType childStateId, StateIdType parentStateId, bool isDefaultState = false)
		{
			if (isDefaultState)
			{
				bool isAdded;
				std::tie(std::ignore, isAdded) = mParentToChildLinks.emplace(parentStateId, childStateId);
				Assert(isAdded, "More than one initial state set for a parent state");
			}
			mChildToParentLinks.emplace(std::forward<StateIdType>(childStateId), std::forward<StateIdType>(parentStateId));
		}

		StateIdType getNextState(const BlackboardType& blackboard, StateIdType previousState) const
		{
			bool needToProcess = true;
			StateIdType currentState = previousState;
			while (needToProcess)
			{
				needToProcess = false;

				bool isProcessedByParent = recursiveUpdateParents(currentState, blackboard);
				if (isProcessedByParent)
				{
					replaceToChildState(currentState);
					needToProcess = true;
					if (currentState == previousState) [[unlikely]]
					{
						ReportError("FSM cycle detected");
						return currentState;
					}
					continue;
				}

				auto stateIt = mStates.find(currentState);
				if (stateIt == mStates.end())
				{
					return currentState;
				}

				for (const LinkPair& link : stateIt->second.links)
				{
					if (link.linkFollowRule->canFollow(blackboard))
					{
						currentState = link.followingState;

						replaceToChildState(currentState);
						needToProcess = true;

						if (currentState == previousState) [[unlikely]]
						{
							ReportError("FSM cycle detected");
							return currentState;
						}
						break;
					}
				}
			}

			return currentState;
		}

	private:
		bool recursiveUpdateParents(StateIdType& state, const BlackboardType& blackboard) const
		{
			auto parentIt = mChildToParentLinks.find(state);
			if (parentIt == mChildToParentLinks.end())
			{
				return false;
			}

			StateIdType processingState = parentIt->second;

			bool isProcessed = recursiveUpdateParents(processingState, blackboard);
			if (isProcessed)
			{
				return true;
			}

			auto stateIt = mStates.find(processingState);
			if (stateIt == mStates.end())
			{
				return false;
			}

			for (const LinkPair& link : stateIt->second.links)
			{
				if (link.linkFollowRule->canFollow(blackboard))
				{
					state = link.followingState;
					return true;
				}
			}

			return false;
		}

		void replaceToChildState(StateIdType& state) const
		{
			typename std::map<StateIdType, StateIdType>::const_iterator childIt;

			while(true)
			{
				childIt = mParentToChildLinks.find(state);
				if (childIt == mParentToChildLinks.end())
				{
					break;
				}
				state = childIt->second;
			}
		}

	private:
		std::map<StateIdType, StateLinkRules> mStates;
		std::map<StateIdType, StateIdType> mChildToParentLinks;
		std::map<StateIdType, StateIdType> mParentToChildLinks;
	};
}

#pragma once

#include <functional>
#include <memory>

#include "GameData/FSM/Blackboard.h"

namespace FSM
{
	template<typename BlackboardKeyType>
	class LinkRule
	{
	public:
		using BlackboardType = Blackboard<BlackboardKeyType>;

	public:
		virtual ~LinkRule() = default;
		virtual bool canFollow(const BlackboardType& blackboard) const = 0;
		virtual std::unique_ptr<LinkRule> makeCopy() const = 0;
	};

	namespace LinkRules
	{
		/**
		 * Link that can be parametrized with functor
		 */
		template<typename BlackboardKeyType>
		class FunctorLink final : public LinkRule<BlackboardKeyType>
		{
		public:
			using BlackboardType = Blackboard<BlackboardKeyType>;
			using CanFollowFn = std::function<bool(const BlackboardType& blackboard)>;

		public:
			explicit FunctorLink(CanFollowFn canFollowFn)
				: mCanFollowFn(canFollowFn)
			{
			}

			bool canFollow(const Blackboard<BlackboardKeyType>& blackboard) const override
			{
				return mCanFollowFn(blackboard);
			}

			std::unique_ptr<LinkRule<BlackboardKeyType>> makeCopy() const override
			{
				return std::make_unique<FunctorLink<BlackboardKeyType>>(mCanFollowFn);
			}

		private:
			CanFollowFn mCanFollowFn;
		};

		/**
		 * Link that will be followed if a blackboard variable have a specific value
		 */
		template<typename BlackboardKeyType, typename ValueType>
		class VariableEqualLink final : public LinkRule<BlackboardKeyType>
		{
		public:
			using BlackboardType = Blackboard<BlackboardKeyType>;

		public:
			template<typename Key, typename Value>
			VariableEqualLink(Key&& name, Value&& expectedValue)
				: mName(std::forward<Key>(name))
				, mExpectedValue(std::forward<Value>(expectedValue))
			{
			}

			bool canFollow(const BlackboardType& blackboard) const override
			{
				return blackboard.template getValue<ValueType>(mName) == mExpectedValue;
			}

			std::unique_ptr<LinkRule<BlackboardKeyType>> makeCopy() const override
			{
				return std::make_unique<VariableEqualLink<BlackboardKeyType, ValueType>>(mName, mExpectedValue);
			}

		private:
			BlackboardKeyType mName;
			ValueType mExpectedValue;
		};
	} // namespace LinkRules
} // namespace FSM

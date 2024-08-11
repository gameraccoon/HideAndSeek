#pragma once

#include <any>
#include <typeindex>
#include <unordered_map>

namespace FSM
{
	template<typename KeyType>
	class Blackboard
	{
	public:
		template<typename T>
		void setValue(KeyType key, std::common_type_t<T> value)
		{
			mValues.insert_or_assign(std::forward<KeyType>(key), std::forward<T>(value));
		}

		template<typename T>
		T getValue(KeyType key) const
		{
			auto it = mValues.find(std::forward<KeyType>(key));
			if (it != mValues.end())
			{
				AssertFatal(it->second.type() == typeid(T), "Wrong blackboard type requested: %s", it->second.type().name());
				return std::any_cast<T>(it->second);
			}

			return T();
		}

		template<typename T>
		T getValue(KeyType key, std::common_type_t<T> def) const
		{
			auto it = mValues.find(std::forward<KeyType>(key));
			if (it != mValues.end())
			{
				AssertFatal(it->second.type() == typeid(T), "Wrong blackboard type requested: %s", it->second.type().name());
				return std::any_cast<T>(it->second);
			}

			return std::forward<T>(def);
		}

	private:
		std::unordered_map<KeyType, std::any> mValues;
	};
} // namespace FSM

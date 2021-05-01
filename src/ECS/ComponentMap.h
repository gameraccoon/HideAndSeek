#pragma once

#include <unordered_map>
#include <algorithm>

namespace Ecs
{
	template <typename ComponentTypeId>
	class ComponentMapImpl
	{
	public:
		using Iterator = std::unordered_map<ComponentTypeId, std::vector<void*>>::iterator;
		using ConstIterator = std::unordered_map<ComponentTypeId, std::vector<void*>>::const_iterator;

	public:
		ComponentMapImpl() = default;
		ComponentMapImpl(const ComponentMapImpl&) = delete;
		ComponentMapImpl& operator=(const ComponentMapImpl&) = delete;
		ComponentMapImpl(ComponentMapImpl&&) = delete;
		ComponentMapImpl& operator=(ComponentMapImpl&&) = delete;
		~ComponentMapImpl() { AssertFatal(mEmptyVector.empty(), "mEmptyVector has changed during runtime, that should never happen"); }

		template<typename FirstComponent, typename... Components>
		[[nodiscard]] auto getComponentVectors()
		{
			auto it = mData.find(FirstComponent::GetTypeName());

			if (it == mData.end())
			{
				return std::tuple_cat(std::tuple<std::vector<void*>&>(mEmptyVector), getEmptyComponentVectors<Components>()...);
			}

			return std::tuple_cat(std::tuple<std::vector<void*>&>(it->second), getComponentVectors<Components>()...);
		}

		[[nodiscard]] std::vector<void*>& getComponentVectorById(ComponentTypeId id)
		{
			auto it = mData.find(id);
			return it == mData.end() ? mEmptyVector : it->second;
		}

		[[nodiscard]] const std::vector<void*>& getComponentVectorById(ComponentTypeId id) const
		{
			return const_cast<ComponentMapImpl*>(this)->getComponentVectorById(id);
		}

		[[nodiscard]] std::vector<void*>& getOrCreateComponentVectorById(ComponentTypeId id)
		{
			return mData[id];
		}

		void cleanEmptyVectors()
		{
			for (auto it = mData.begin(), itEnd = mData.end(); it != itEnd;)
			{
				if (it->second.empty())
				{
					it = mData.erase(it);
				}
				else
				{
					++it;
				}
			}
			AssertFatal(mEmptyVector.empty(), "mEmptyVector should be empty");
		}

		[[nodiscard]] Iterator begin() noexcept { return mData.begin(); }
		[[nodiscard]] Iterator end() noexcept { return mData.end(); }
		[[nodiscard]] ConstIterator begin() const noexcept { return mData.cbegin(); }
		[[nodiscard]] ConstIterator end() const noexcept { return mData.cend(); }

	private:
		template<int I = 0>
		std::tuple<> getEmptyComponentVectors()
		{
			return std::tuple<>();
		}

		template<typename FirstComponent, typename... Components>
		auto getEmptyComponentVectors()
		{
			return std::tuple_cat(std::tuple<std::vector<void*>&>(mEmptyVector), getEmptyComponentVectors<Components...>());
		}

		template<int I = 0>
		std::tuple<> getComponentVectors()
		{
			return std::tuple<>();
		}

	private:
		std::unordered_map<ComponentTypeId, std::vector<void*>> mData;
		std::vector<void*> mEmptyVector;
	};

} // namespace Ecs

#pragma once

#include <array>
#include <numeric>
#include <vector>

#include "ErrorHandling.h"

namespace Ecs
{
	class ComponentPoolBase
	{
	public:
		virtual ~ComponentPoolBase() = default;
	};

	template <typename ComponentType, std::size_t PageSize>
	class ComponentPool final : public ComponentPoolBase
	{
	public:
		ComponentPool(size_t preallocatedPages) noexcept
		{
			for (size_t i = 0; i < preallocatedPages; ++i)
			{
				allocateNewPage();
			}
		}

		~ComponentPool() final
		{
			// we assume that all the components were unregistered before component pool destruction
			for (Page* page : mPages)
			{
				delete page;
			}
		}

		ComponentPool(ComponentPool&) = delete;
		ComponentPool& operator=(ComponentPool&) = delete;
		ComponentPool(ComponentPool&&) = delete;
		ComponentPool& operator=(ComponentPool&&) = delete;

		void* acquireComponent()
		{
			if (mNextFreeSlot == nullptr)
			{
				allocateNewPage();
			}

			ComponentSlot* takenSlot = mNextFreeSlot;
			mNextFreeSlot = takenSlot->nextFreeSlot;

			return new (&takenSlot->component) ComponentType;
		}

		void releaseComponent(void* component)
		{
			// component is a union part of CompoenentSlot, so they have the same address in memory
			ComponentSlot* slot = static_cast<ComponentSlot*>(component);
			slot->component.~ComponentType();
			slot->nextFreeSlot = mNextFreeSlot;
			mNextFreeSlot = slot;
		}

	private:
		struct ComponentSlot {
			union {
				// free list implementation
				ComponentType component;
				ComponentSlot* nextFreeSlot;
			};

			ComponentSlot()
			{
			}

			~ComponentSlot()
			{
				// do nothing. assume that the component was released before destroing the pool
			}
		};

		using Page = std::array<ComponentSlot, PageSize>;

	private:
		void allocateNewPage()
		{
			mPages.push_back(new (std::nothrow) Page);

			Page& newPage = *mPages.back();
			for (size_t i = 0; i < PageSize - 1; ++i)
			{
				newPage[i].nextFreeSlot = &newPage[i + 1];
			}

			newPage[PageSize - 1].nextFreeSlot = mNextFreeSlot;
			mNextFreeSlot = &newPage[0];
		}

	private:
		ComponentSlot* mNextFreeSlot = nullptr;
		std::vector<Page*> mPages;
	};

} // namespace Ecs

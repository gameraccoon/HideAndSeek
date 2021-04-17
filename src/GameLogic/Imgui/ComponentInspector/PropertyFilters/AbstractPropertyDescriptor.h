#pragma once

#include <memory>
#include <any>

#include "ECS/Entity.h"

class EntityManager;

namespace ImguiPropertyFiltration
{
	class AbstractPropertyFilterFactory;

	class AbstractPropertyDescriptor
	{
	public:
		explicit AbstractPropertyDescriptor(const std::string& name) : mName(name) {}
		virtual ~AbstractPropertyDescriptor() = default;

		[[nodiscard]] virtual std::any getPropertyValue(EntityManager& entityManager, Entity entity) = 0;
		[[nodiscard]] virtual StringId getComponentType() const = 0;

		[[nodiscard]] const std::string& getName() const { return mName; }
		[[nodiscard]] const std::vector<std::shared_ptr<AbstractPropertyFilterFactory>>& getFilterFactories() const { return mFilterFactories; }
		[[nodiscard]] bool hasFactories() const { return !mFilterFactories.empty(); }

	protected:
		std::vector<std::shared_ptr<AbstractPropertyFilterFactory>> mFilterFactories;

	private:
		std::string mName;
	};
} // namespace ImguiPropertyFiltration

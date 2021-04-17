#include "Base/precomp.h"

#include "ECS/ComponentFactory.h"

ComponentFactory::CreationFn ComponentFactory::getCreationFn(StringId className) const
{
	const auto& it = mComponentCreators.find(className);
	if (it != mComponentCreators.cend())
	{
		return it->second;
	}

	ReportFatalError("Unknown component type: '%s'", className);
	return nullptr;
}

BaseComponent* ComponentFactory::createComponent(StringId typeName) const
{
	const auto& it = mComponentCreators.find(typeName);
	if (it != mComponentCreators.cend() && it->second)
	{
		return it->second();
	}

	return nullptr;
}

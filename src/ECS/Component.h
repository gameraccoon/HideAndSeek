#pragma once

/**
 * The base class for actor components
 *
 * Abstract
 */
class BaseComponent
{
public:
	virtual ~BaseComponent() = default;

	[[nodiscard]] virtual StringId getComponentTypeName() const = 0;
};

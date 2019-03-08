#pragma once

#include <memory>

#include "Core/ActorComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Vector2D.h"

/**
 * Component that stores information about a light emitter
 */
class LightComponent : public ActorComponent
{
public:
	using Ptr = std::shared_ptr<LightComponent>;
	using WeakPtr = std::weak_ptr<LightComponent>;

public:
	virtual ~LightComponent() = default;

	float getBrightness() const;
	void setBrightness(float newSize);

	TransformComponent::WeakPtr getTransformComponent() const;
	void setTransformComponent(TransformComponent::WeakPtr newTransformComponent);

private:
	float mBrightness;

	TransformComponent::WeakPtr mTransformComponent;
};

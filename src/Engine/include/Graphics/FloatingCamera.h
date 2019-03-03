#pragma once

#include "Camera.h"

class FloatingCamera : public Camera
{
public:
	FloatingCamera(const World* world, Vector2D resolution, Vector2D location);
	~FloatingCamera() override;
	/** Set new location of this camera in the world */
	virtual void setLocation(const Vector2D &newLocation) override;
	/** Set shift of center of the screen */
	void setCenterShift(Vector2D shift);
protected:
	void renderFog();

	Vector2D mShift;
};
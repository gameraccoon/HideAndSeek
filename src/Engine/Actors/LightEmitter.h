#pragma once

#include "Actor.h"

class LightEmitter : public Actor
{
public:
	LightEmitter(World *world, Vector2D location, Vector2D scale, Rotator rotation);
	~LightEmitter();

	float getBrightness() const;
	void setBrightness(float brightness);

protected:
	float mBrightness;
	long mColor; // 0xAARRGGBB
};

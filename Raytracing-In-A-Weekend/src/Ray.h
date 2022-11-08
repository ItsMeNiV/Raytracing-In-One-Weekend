#pragma once
#include "Vec3.h"

class Ray
{
public:
	Ray() = default;
	Ray(const Vec3& origin, const Vec3& direction)
		:origin(origin), direction(direction)
	{}

	Vec3 At(double t) const { return origin + t * direction; }

public:
	Vec3 origin, direction;
};
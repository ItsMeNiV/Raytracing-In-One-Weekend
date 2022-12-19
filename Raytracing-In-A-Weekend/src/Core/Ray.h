#pragma once
#include "glm/glm.hpp"

class Ray
{
public:
	Ray() = default;
	Ray(const glm::vec3& origin, const glm::vec3& direction)
		:origin(origin), direction(direction)
	{}

	glm::vec3 At(float t) const { return origin + t * direction; }

public:
	glm::vec3 origin, direction;
};
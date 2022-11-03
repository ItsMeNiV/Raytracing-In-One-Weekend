#pragma once
#include "RTWeekend.h"

class Camera
{
public:
	Camera()
	{
		auto aspectRatio = 16.0f / 9.0f;
		auto viewportHeight = 2.0f;
		auto viewportWidth = aspectRatio * viewportHeight;
		auto focalLength = 1.0f;

		origin = glm::vec3(0.0f, 0.0f, 0.0f);
		horizontal = glm::vec3(viewportWidth, 0.0f, 0.0f);
		vertical = glm::vec3(0.0, viewportHeight, 0.0f);
		lowerLeftCorner = origin - horizontal / 2.0f - vertical / 2.0f - glm::vec3(0.0f, 0.0f, focalLength);
	}

	Ray GetRay(float u, float v) const
	{
		return Ray(origin, lowerLeftCorner + u * horizontal + v * vertical - origin);
	}

private:
	glm::vec3 origin, lowerLeftCorner, horizontal, vertical;

};
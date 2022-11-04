#pragma once
#include "RTWeekend.h"

class Camera
{
public:
	Camera(glm::vec3 lookfrom, glm::vec3 lookat, glm::vec3 vup, float vfov, float aspectRatio)
	{
		auto theta = glm::radians(vfov);
		auto h = tan(theta / 2.0f);
		auto viewportHeight = 2.0f * h;
		auto viewportWidth = aspectRatio * viewportHeight;
		auto focalLength = 1.0f;

		auto w = unitVector(lookfrom - lookat);
		auto u = unitVector(glm::cross(vup, w));
		auto v = glm::cross(w, u);

		origin = lookfrom;
		horizontal = viewportWidth * u;
		vertical = viewportHeight * v;
		lowerLeftCorner = origin - horizontal / 2.0f - vertical / 2.0f - w;
	}

	Ray GetRay(float s, float t) const
	{
		return Ray(origin, lowerLeftCorner + s * horizontal + t * vertical - origin);
	}

private:
	glm::vec3 origin, lowerLeftCorner, horizontal, vertical;

};
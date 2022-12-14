#pragma once
#include "Core/RTWeekend.h"

class ONB
{
public:
	ONB() = default;

	inline glm::vec3 operator[](int i) const { return axis[i]; }

	glm::vec3 u() const { return axis[0]; }
	glm::vec3 v() const { return axis[1]; }
	glm::vec3 w() const { return axis[2]; }

	glm::vec3 local(float a, float b, float c) const
	{
		return a * u() + b * v() + c * w();
	}

	glm::vec3 local(const glm::vec3& a) const
	{
		return a.x * u() + a.y * v() + a.z * w();
	}

	void buildFromW(const glm::vec3& n);

public:
	glm::vec3 axis[3];

};
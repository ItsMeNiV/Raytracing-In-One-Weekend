#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <cmath>
#include <limits>
#include <memory>
#include <cstdlib>
#include <random>

// Usings
using std::sqrt;

// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Common Headers

#include "Ray.h"
#include "glm/glm.hpp"


inline float vecLengthSquared(const glm::vec3& vec)
{
	return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
}

inline float vecDot(const glm::vec3& u, const glm::vec3& v)
{
	return u.x * v.x
		+ u.y * v.y
		+ u.z * v.z;
}

inline float vecLength(const glm::vec3& vec)
{
	return sqrt(vecLengthSquared(vec));
}

inline float randomFloat()
{
	static std::uniform_real_distribution<double> distribution(0.0, 1.0);
	static std::mt19937 generator;
	return distribution(generator);
}

inline float randomFloat(float min, float max)
{
	return min + (max - min) * randomFloat();
}

inline float clamp(float x, float min, float max)
{
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

inline glm::vec3 randomVec()
{
	return glm::vec3(randomFloat(), randomFloat(), randomFloat());
}

inline glm::vec3 randomVec(float min, float max)
{
	return glm::vec3(randomFloat(min, max), randomFloat(min, max), randomFloat(min, max));
}

glm::vec3 randomVecInUnitSphere()
{
	while (true)
	{
		glm::vec3 p = randomVec(-1.0f, 1.0f);
		if (glm::dot(p, p) >= 1.0f)
			continue;
		return p;
	}
}

glm::vec3 unitVector(const glm::vec3 vec)
{
	return vec / vecLength(vec);
}

glm::vec3 randomUnitVector()
{
	glm::vec3 vec = randomVecInUnitSphere();
	return unitVector(vec);
}

glm::vec3 randomInHemisphere(const glm::vec3& normal)
{
	glm::vec3 inUnitSphere = randomVecInUnitSphere();
	if (vecDot(inUnitSphere, normal) > 0.0f)
		return inUnitSphere;
	else
		return -inUnitSphere;
}

glm::vec3 randomInUnitDisk()
{
	while (true)
	{
		glm::vec3 p = glm::vec3(randomFloat(-1.0f, 1.0f), randomFloat(-1.0f, 1.0f), 0.0f);
		if (vecLengthSquared(p) >= 1) continue;
		return p;
	}
}

bool vecNearZero(const glm::vec3& vec)
{
	const auto s = 1e-8;
	return (fabs(vec.x) < s) && (fabs(vec.y) < s) && (fabs(vec.z) < s);
}

glm::vec3 refract(const glm::vec3& uv, const glm::vec3& n, float etaiOverEtat)
{
	float cosTheta = fmin(vecDot(-uv, n), 1.0);
	glm::vec3 rOutPerp = etaiOverEtat * (uv + cosTheta * n);
	glm::vec3 rOutParallel = -sqrt(fabs(1.0f - vecLengthSquared(rOutPerp))) * n;
	return rOutPerp + rOutParallel;
}

#endif
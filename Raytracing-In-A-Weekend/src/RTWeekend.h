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

const float infinity = std::numeric_limits<float>::infinity();
const float pi = 3.1415926535897932385;

// Common Headers
#include "glm/glm.hpp"
#include "Ray.h"

inline float degreesToRadians(float degrees) {
	return degrees * pi / 180.0f;
}

inline float randomfloat()
{
	static std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
	static std::mt19937 generator;
	return distribution(generator);
}

inline float randomfloat(float min, float max)
{
	return min + (max - min) * randomfloat();
}

inline float clamp(float x, float min, float max)
{
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

inline glm::vec3 randomVec()
{
	return glm::vec3(randomfloat(), randomfloat(), randomfloat());
}

inline glm::vec3 randomVec(float min, float max)
{
	return glm::vec3(randomfloat(min, max), randomfloat(min, max), randomfloat(min, max));
}

glm::vec3 randomVecInUnitSphere();

glm::vec3 randomUnitVector();

glm::vec3 randomInHemisphere(const glm::vec3& normal);

glm::vec3 randomInUnitDisk();

bool vecNearZero(const glm::vec3& vec);

glm::vec3 refract(const glm::vec3& uv, const glm::vec3& n, float etaiOverEtat);

#endif
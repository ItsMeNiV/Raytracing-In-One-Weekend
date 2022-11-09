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
#include "Vec3.h"
#include "Ray.h"

inline double degreesToRadians(double degrees) {
	return degrees * pi / 180.0;
}

inline double randomdouble()
{
	static std::uniform_real_distribution<double> distribution(0.0, 1.0);
	static std::mt19937 generator;
	return distribution(generator);
}

inline double randomdouble(double min, double max)
{
	return min + (max - min) * randomdouble();
}

inline double clamp(double x, double min, double max)
{
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

inline Vec3 randomVec()
{
	return Vec3(randomdouble(), randomdouble(), randomdouble());
}

inline Vec3 randomVec(double min, double max)
{
	return Vec3(randomdouble(min, max), randomdouble(min, max), randomdouble(min, max));
}

Vec3 randomVecInUnitSphere();

Vec3 randomUnitVector();

Vec3 randomInHemisphere(const Vec3& normal);

Vec3 randomInUnitDisk();

bool vecNearZero(const Vec3& vec);

Vec3 refract(const Vec3& uv, const Vec3& n, double etaiOverEtat);

#endif
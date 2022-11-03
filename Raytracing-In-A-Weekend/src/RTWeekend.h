#pragma once
#include <cmath>
#include <limits>
#include <memory>
#include <cstdlib>


// Usings

using std::shared_ptr;
using std::make_shared;
using std::sqrt;

// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Common Headers

#include "Ray.h"
#include "glm/glm.hpp"


inline float randomFloat()
{
	return rand() / (RAND_MAX + 1.0);
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
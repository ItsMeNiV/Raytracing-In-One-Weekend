#include "Core/RTWeekend.h"

glm::vec3 randomVecInUnitSphere()
{
	while (true)
	{
		glm::vec3 p = randomVec(-1.0f, 1.0f);
		if (dot(p, p) >= 1.0f)
			continue;
		return p;
	}
}

glm::vec3 randomUnitVector()
{
	glm::vec3 vec = randomVecInUnitSphere();
	return glm::normalize(vec);
}

glm::vec3 randomInHemisphere(const glm::vec3& normal)
{
	glm::vec3 inUnitSphere = randomVecInUnitSphere();
	if (dot(inUnitSphere, normal) > 0.0f)
		return inUnitSphere;
	else
		return -inUnitSphere;
}

glm::vec3 randomInUnitDisk()
{
	while (true)
	{
		glm::vec3 p = glm::vec3(randomFloat(-1.0f, 1.0f), randomFloat(-1.0f, 1.0f), 0.0f);
		if (glm::dot(p, p) >= 1) continue;
		return p;
	}
}

glm::vec3 randomCosineDirection() {
	auto r1 = randomFloat();
	auto r2 = randomFloat();
	auto z = sqrt(1 - r2);

	auto phi = 2 * pi * r1;
	auto x = cos(phi) * sqrt(r2);
	auto y = sin(phi) * sqrt(r2);

	return glm::vec3(x, y, z);
}

bool vecNearZero(const glm::vec3& vec)
{
	const auto s = 1e-8;
	return (fabs(vec.x) < s) && (fabs(vec.y) < s) && (fabs(vec.z) < s);
}

glm::vec3 refract(const glm::vec3& uv, const glm::vec3& n, float etaiOverEtat)
{
	float cosTheta = fmin(dot(-uv, n), 1.0f);
	glm::vec3 rOutPerp = etaiOverEtat * (uv + cosTheta * n);
	glm::vec3 rOutParallel = -sqrt(fabs(1.0f - glm::dot(rOutPerp, rOutPerp))) * n;
	return rOutPerp + rOutParallel;
}
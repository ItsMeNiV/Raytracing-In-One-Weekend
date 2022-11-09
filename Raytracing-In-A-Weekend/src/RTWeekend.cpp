#include "RTWeekend.h"

Vec3 randomVecInUnitSphere()
{
	while (true)
	{
		Vec3 p = randomVec(-1.0, 1.0);
		if (dot(p, p) >= 1.0)
			continue;
		return p;
	}
}

Vec3 randomUnitVector()
{
	Vec3 vec = randomVecInUnitSphere();
	return unitVector(vec);
}

Vec3 randomInHemisphere(const Vec3& normal)
{
	Vec3 inUnitSphere = randomVecInUnitSphere();
	if (dot(inUnitSphere, normal) > 0.0)
		return inUnitSphere;
	else
		return -inUnitSphere;
}

Vec3 randomInUnitDisk()
{
	while (true)
	{
		Vec3 p = Vec3(randomdouble(-1.0, 1.0), randomdouble(-1.0, 1.0), 0.0);
		if (p.length_squared() >= 1) continue;
		return p;
	}
}

bool vecNearZero(const Vec3& vec)
{
	const auto s = 1e-8;
	return (fabs(vec.x) < s) && (fabs(vec.y) < s) && (fabs(vec.z) < s);
}

Vec3 refract(const Vec3& uv, const Vec3& n, double etaiOverEtat)
{
	double cosTheta = fmin(dot(-uv, n), 1.0);
	Vec3 rOutPerp = etaiOverEtat * (uv + cosTheta * n);
	Vec3 rOutParallel = -sqrt(fabs(1.0 - rOutPerp.length_squared())) * n;
	return rOutPerp + rOutParallel;
}
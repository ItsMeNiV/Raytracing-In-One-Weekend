#pragma once

class Camera
{
public:
	Camera(Vec3 lookfrom, Vec3 lookat, Vec3 vup, double vfov, double aspectRatio, double aperture, double focusDist)
	{
		auto theta = degreesToRadians(vfov);
		auto h = tan(theta / 2.0);
		auto viewportHeight = 2.0 * h;
		auto viewportWidth = aspectRatio * viewportHeight;
		auto focalLength = 1.0;

		w = unitVector(lookfrom - lookat);
		u = unitVector(cross(vup, w));
		v = cross(w, u);

		origin = lookfrom;
		horizontal = focusDist * viewportWidth * u;
		vertical = focusDist * viewportHeight * v;
		lowerLeftCorner = origin - horizontal / 2.0 - vertical / 2.0 - focusDist * w;

		lensRadius = aperture / 2;
	}

	Ray GetRay(double s, double t) const
	{
		Vec3 rd = lensRadius * randomInUnitDisk();
		Vec3 offset = u * rd.x + v * rd.y;

		return Ray(origin + offset, lowerLeftCorner + s * horizontal + t * vertical - origin - offset);
	}

private:
	Vec3 origin, lowerLeftCorner, horizontal, vertical;
	Vec3 u, v, w;
	double lensRadius;

};
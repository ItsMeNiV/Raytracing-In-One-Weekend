#pragma once

class Camera
{
public:
	Camera(glm::vec3 lookfrom, glm::vec3 lookat, glm::vec3 vup, float vfov, float aspectRatio, float aperture, float focusDist)
	{
		float theta = glm::radians(vfov);
		float h = glm::tan(theta / 2.0f);
		float viewportHeight = 2.0f * h;
		float viewportWidth = aspectRatio * viewportHeight;
		float focalLength = 1.0f;

		w = glm::normalize(lookfrom - lookat);
		u = glm::normalize(glm::cross(vup, w));
		v = glm::cross(w, u);

		origin = lookfrom;
		horizontal = focusDist * viewportWidth * u;
		vertical = focusDist * viewportHeight * v;
		lowerLeftCorner = origin - horizontal / 2.0f - vertical / 2.0f - focusDist * w;

		lensRadius = aperture / 2;
	}

	Ray GetRay(float s, float t) const
	{
		glm::vec3 rd = lensRadius * randomInUnitDisk();
		glm::vec3 offset = u * rd.x + v * rd.y;

		return Ray(origin + offset, lowerLeftCorner + s * horizontal + t * vertical - origin - offset);
	}

private:
	glm::vec3 origin, lowerLeftCorner, horizontal, vertical;
	glm::vec3 u, v, w;
	float lensRadius;

};
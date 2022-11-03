﻿#include <iostream>
#include "RTWeekend.h"
#include "Hittable.h"
#include "Camera.h"

glm::vec3 rayColor(const Ray& r, const Hittable& world)
{
	HitRecord rec;
	if (world.Hit(r, 0, infinity, rec))
	{
		return 0.5f * (rec.normal + glm::vec3(1.0f, 1.0f, 1.0f));
	}
	glm::vec3 unitDirection = glm::normalize(r.direction);
	float t = 0.5f * (unitDirection.y + 1.0f);
	return (1.0f - t) * glm::vec3(1.0f, 1.0f, 1.0f) + t * glm::vec3(0.5f, 0.7f, 1.0f);
}

void writeColor(std::ostream& out, glm::vec3 pixelColor, int samplesPerPixel) {
	auto r = pixelColor.x;
	auto g = pixelColor.y;
	auto b = pixelColor.z;

	auto scale = 1.0 / samplesPerPixel;
	r *= scale;
	g *= scale;
	b *= scale;

	out << static_cast<int>(255.999 * clamp(r, 0.0, 0.999)) << ' '
		<< static_cast<int>(255.999 * clamp(g, 0.0, 0.999)) << ' '
		<< static_cast<int>(255.999 * clamp(b, 0.0, 0.999)) << '\n';
}

int main()
{
	//Image
	const float aspectRatio = 16.0f / 9.0f;
	const int imageWidth = 400;
	const int imageHeight = static_cast<int>(imageWidth / aspectRatio);
	const int samplesPerPixel = 100;

	//World
	HittableList world;
	world.add(make_shared<Sphere>(glm::vec3(0.0f, 0.0f, -1.0f), 0.5f));
	world.add(make_shared<Sphere>(glm::vec3(0.0f, -100.5f, -1.0f), 100.0f));

	//Camera
	Camera cam;

	//Render
	std::cout << "P3\n" << imageWidth << ' ' << imageHeight << "\n255\n";

	for (int j = imageHeight -1; j >= 0; --j)
	{
		std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < imageWidth; ++i)
		{
			glm::vec3 pixelColor(0.0f, 0.0f, 0.0f);
			for (int s = 0; s < samplesPerPixel; ++s)
			{
				float u = (i + randomFloat()) / (imageWidth - 1);
				float v = (j + randomFloat()) / (imageHeight - 1);
				Ray r = cam.GetRay(u, v);
				pixelColor += rayColor(r, world);
			}
			writeColor(std::cout, pixelColor, samplesPerPixel);
		}
	}

	std::cerr << "\nDone.\n";
}

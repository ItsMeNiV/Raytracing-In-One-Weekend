#include <iostream>
#include <fstream>
#include "RTWeekend.h"
#include "Hittable.h"
#include "Camera.h"
#include "Material.h"

glm::vec3 rayColor(const Ray& r, const Hittable& world, int depth)
{
	HitRecord rec;

	if (depth <= 0)
		return glm::vec3(0.0f, 0.0f, 0.0f);

	if (world.Hit(r, 0.001f, infinity, rec))
	{
		Ray scattered;
		glm::vec3 attenuation;
		if (rec.matPtr->scatter(r, rec, attenuation, scattered))
			return attenuation * rayColor(scattered, world, depth - 1);
		return glm::vec3(0.0f, 0.0f, 0.0f);
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
	r = sqrt(scale * r);
	g = sqrt(scale * g);
	b = sqrt(scale * b);

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
	const int maxDepth = 50;

	//Materials
	auto material_ground = std::make_shared<Lambertian>(glm::vec3(0.8f, 0.8f, 0.0f));
	auto material_center = std::make_shared<Lambertian>(glm::vec3(0.1f, 0.2f, 0.5f));
	auto material_left = std::make_shared<Dielectric>(1.5f);
	auto material_right = std::make_shared<Metal>(glm::vec3(0.8f, 0.6f, 0.2f), 0.0f);

	//World
	HittableList world;
	world.add(std::make_shared<Sphere>(glm::vec3(0.0f, -100.5f, -1.0f), 100.0f, material_ground));
	world.add(std::make_shared<Sphere>(glm::vec3(0.0f, 0.0f, -1.0f), 0.5f, material_center));
	world.add(std::make_shared<Sphere>(glm::vec3(-1.0f, 0.0f, -1.0f), 0.5f, material_left));
	world.add(std::make_shared<Sphere>(glm::vec3(-1.0f, 0.0f, -1.0f), -0.4f, material_left));
	world.add(std::make_shared<Sphere>(glm::vec3(1.0f, 0.0f, -1.0f), 0.5f, material_right));

	//Camera
	Camera cam;

	//Render
	std::ofstream fout("image.ppm");
	std::cout.rdbuf(fout.rdbuf());
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
				pixelColor += rayColor(r, world, maxDepth);
			}
			writeColor(std::cout, pixelColor, samplesPerPixel);
		}
	}

	std::cerr << "\nDone.\n";
}

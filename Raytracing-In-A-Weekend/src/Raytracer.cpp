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

HittableList randomScene() {
	HittableList world;

	auto groundMaterial = std::make_shared<Lambertian>(glm::vec3(0.5f, 0.5f, 0.5f));
	world.add(std::make_shared<Sphere>(glm::vec3(0.0f, -1000.0f, 0.0f), 1000.0f, groundMaterial));

	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			auto chooseMat = randomFloat();
			glm::vec3 center(a + 0.9f * randomFloat(), 0.2f, b + 0.9f * randomFloat());

			if ((center - glm::vec3(4.0f, 0.2f, 0.0f)).length() > 0.9f) {
				shared_ptr<Material> sphereMaterial;

				if (chooseMat < 0.8) {
					// diffuse
					auto albedo = randomVec() * randomVec();
					sphereMaterial = std::make_shared<Lambertian>(albedo);
					world.add(std::make_shared<Sphere>(center, 0.2f, sphereMaterial));
				}
				else if (chooseMat < 0.95f) {
					// metal
					auto albedo = randomVec(0.5f, 1.0f);
					auto fuzz = randomFloat(0.0f, 0.5f);
					sphereMaterial = std::make_shared<Metal>(albedo, fuzz);
					world.add(std::make_shared<Sphere>(center, 0.2f, sphereMaterial));
				}
				else {
					// glass
					sphereMaterial = std::make_shared<Dielectric>(1.5f);
					world.add(std::make_shared<Sphere>(center, 0.2f, sphereMaterial));
				}
			}
		}
	}

	auto material1 = make_shared<Dielectric>(1.5f);
	world.add(make_shared<Sphere>(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f, material1));

	auto material2 = make_shared<Lambertian>(glm::vec3(0.4f, 0.2f, 0.1f));
	world.add(make_shared<Sphere>(glm::vec3(-4.0f, 1.0f, 0.0f), 1.0f, material2));

	auto material3 = make_shared<Metal>(glm::vec3(0.7f, 0.6f, 0.5f), 0.0f);
	world.add(make_shared<Sphere>(glm::vec3(4.0f, 1.0f, 0.0f), 1.0f, material3));

	return world;
}

int main()
{
	//Image
	const float aspectRatio = 3.0f / 2.0f;
	const int imageWidth = 1200;
	const int imageHeight = static_cast<int>(imageWidth / aspectRatio);
	const int samplesPerPixel = 500;
	const int maxDepth = 50;

	//World
	HittableList world = randomScene();


	//Camera
	glm::vec3 lookfrom = { 13.0f, 2.0f, 3.0f };
	glm::vec3 lookat = { 0.0f, 0.0f, 0.0f };
	glm::vec3 vup = { 0.0f, 1.0f, 0.0f };
	float distToFocus = 10.0f;
	float aperture = 0.1f;
	Camera cam(lookfrom, lookat, vup, 20.0f, aspectRatio, aperture, distToFocus);

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

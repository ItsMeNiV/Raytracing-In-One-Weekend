#pragma once
#include <iostream>
#include <fstream>
#include "Hittable.h"
#include "Camera.h"
#include "Material.h"
#include "RTWeekend.h"

class Raytracer
{
public:
	Raytracer(std::ofstream& fout, Camera camera, HittableList& world, const int imageHeight, const int imageWidth, const int samplesPerPixel, const int maxDepth)
		: mCamera(camera), mWorld(world), mImageHeight(imageHeight), mImageWidth(imageWidth), mSamplesPerPixel(samplesPerPixel), mMaxDepth(maxDepth)
	{
		std::cout.rdbuf(fout.rdbuf());
	}
	
	virtual void run() = 0;

protected:
	Camera& mCamera;
	HittableList& mWorld;
	int mImageHeight, mImageWidth, mSamplesPerPixel, mMaxDepth;

	glm::vec3 rayColor(const Ray& r, int depth)
	{
		HitRecord rec;

		if (depth <= 0)
			return glm::vec3(0.0f, 0.0f, 0.0f);

		if (mWorld.Hit(r, 0.001f, infinity, rec))
		{
			Ray scattered;
			glm::vec3 attenuation;
			if (rec.matPtr->scatter(r, rec, attenuation, scattered))
				return attenuation * rayColor(scattered, depth - 1);
			return glm::vec3(0.0f, 0.0f, 0.0f);
		}
		glm::vec3 unitDirection = glm::normalize(r.direction);
		float t = 0.5f * (unitDirection.y + 1.0f);
		return (1.0f - t) * glm::vec3(1.0f, 1.0f, 1.0f) + t * glm::vec3(0.5f, 0.7f, 1.0f);
	}

	void writeColor(glm::vec3 pixelColor, int samplesPerPixel)
	{
		auto r = pixelColor.x;
		auto g = pixelColor.y;
		auto b = pixelColor.z;

		auto scale = 1.0 / samplesPerPixel;
		r = sqrt(scale * r);
		g = sqrt(scale * g);
		b = sqrt(scale * b);

		std::cout << static_cast<int>(255.999 * clamp(r, 0.0, 0.999)) << ' '
			<< static_cast<int>(255.999 * clamp(g, 0.0, 0.999)) << ' '
			<< static_cast<int>(255.999 * clamp(b, 0.0, 0.999)) << '\n';
	}

};

class RaytracerNormal : public Raytracer
{
public:
	RaytracerNormal(std::ofstream& fout, Camera camera, HittableList& world, const int imageHeight, const int imageWidth, const int samplesPerPixel, const int maxDepth)
		: Raytracer(fout, camera, world, imageHeight, imageWidth, samplesPerPixel, maxDepth) {}

	virtual void run() override;
};

class RaytracerMT : public Raytracer
{
public:
	RaytracerMT(std::ofstream& fout, Camera camera, HittableList& world, const int imageHeight, const int imageWidth, const int samplesPerPixel, const int maxDepth)
		: Raytracer(fout, camera, world, imageHeight, imageWidth, samplesPerPixel, maxDepth) {}

	virtual void run() override;
};
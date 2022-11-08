#pragma once
#include <iostream>
#include <fstream>
#include <map>
#include <mutex>
#include <string>
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

	Vec3 rayColor(const Ray& r, int depth)
	{
		HitRecord rec;

		if (depth <= 0)
			return Vec3(0.0, 0.0, 0.0);

		if (mWorld.Hit(r, 0.001, infinity, rec))
		{
			Ray scattered;
			Vec3 attenuation;
			if (rec.matPtr->scatter(r, rec, attenuation, scattered))
				return attenuation * rayColor(scattered, depth - 1);
			return Vec3(0.0, 0.0, 0.0);
		}
		Vec3 unitDirection = unitVector(r.direction);
		double t = 0.5 * (unitDirection.y + 1.0);
		return (1.0 - t) * Vec3(1.0, 1.0, 1.0) + t * Vec3(0.5, 0.7, 1.0);
	}

	void writeColor(Vec3 pixelColor, int samplesPerPixel)
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

	void writeColor(Vec3 pixelColor, int samplesPerPixel, std::string& outputString)
	{
		auto r = pixelColor.x;
		auto g = pixelColor.y;
		auto b = pixelColor.z;

		auto scale = 1.0 / samplesPerPixel;
		r = sqrt(scale * r);
		g = sqrt(scale * g);
		b = sqrt(scale * b);

		outputString += std::to_string(static_cast<int>(255.999 * clamp(r, 0.0, 0.999))) + ' '
			+ std::to_string(static_cast<int>(255.999 * clamp(g, 0.0, 0.999))) + ' '
			+ std::to_string(static_cast<int>(255.999 * clamp(b, 0.0, 0.999))) + '\n';
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

private:
	std::map<int, std::string> mOutputStringMap;
	std::mutex mOutputStringMapMutex;

	void writeLines(std::vector<int> lineNumbers);
};
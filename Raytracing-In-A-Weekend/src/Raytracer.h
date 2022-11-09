#pragma once
#include <iostream>
#include <fstream>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <array>
#include "glad/glad.h"
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
	Raytracer(Camera camera, HittableList& world, const int imageHeight, const int imageWidth, const int samplesPerPixel, const int maxDepth)
		: mCamera(camera), mWorld(world), mImageHeight(imageHeight), mImageWidth(imageWidth), mSamplesPerPixel(samplesPerPixel), mMaxDepth(maxDepth)
	{}
	
	virtual void run() = 0;

protected:
	Camera& mCamera;
	HittableList& mWorld;
	int mImageHeight, mImageWidth, mSamplesPerPixel, mMaxDepth;

	Vec3 rayColor(const Ray& r, int depth);

	void writeColor(Vec3 pixelColor, int samplesPerPixel);

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
	RaytracerMT(std::shared_ptr<std::array<std::array<std::array<GLubyte, 3>, 800>, 600>> imageTextureData, Camera camera, HittableList& world, const int imageHeight, const int imageWidth, const int samplesPerPixel, const int maxDepth)
		: Raytracer(camera, world, imageHeight, imageWidth, samplesPerPixel, maxDepth), mImageTextureData(imageTextureData) {}

	virtual void run() override;

private:
	std::shared_ptr<std::array<std::array<std::array<GLubyte, 3>, 800>, 600>> mImageTextureData;
	std::mutex mOutputMutex;

	void writeLines(std::vector<int> lineNumbers);
};

HittableList randomScene();
#pragma once
#include <iostream>
#include <fstream>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include "glad/glad.h"
#include "Hittable.h"
#include "Camera.h"
#include "Material.h"
#include "RTWeekend.h"

class Raytracer
{
public:
	Raytracer(std::shared_ptr<std::vector<GLubyte>> imageTextureData, Camera& camera, HittableList& world, const int imageHeight, const int imageWidth, const int samplesPerPixel, const int maxDepth)
		: mImageTextureData(imageTextureData), mCamera(camera), mWorld(world), mImageHeight(imageHeight), mImageWidth(imageWidth), mSamplesPerPixel(samplesPerPixel), mMaxDepth(maxDepth)
	{}
	
	virtual void run() = 0;

protected:
	std::shared_ptr<std::vector<GLubyte>> mImageTextureData;
	Camera& mCamera;
	HittableList& mWorld;
	int mImageHeight, mImageWidth, mSamplesPerPixel, mMaxDepth;

	Vec3 rayColor(const Ray& r, int depth);

	void writeColor(Vec3 pixelColor, int samplesPerPixel, int lineNumber, int columnNumber);

};

class RaytracerNormal : public Raytracer
{
public:
	RaytracerNormal(std::shared_ptr<std::vector<GLubyte>> imageTextureData, Camera& camera, HittableList& world, const int imageHeight, const int imageWidth, const int samplesPerPixel, const int maxDepth)
		: Raytracer(imageTextureData, camera, world, imageHeight, imageWidth, samplesPerPixel, maxDepth) {}

	virtual void run() override;
};

class RaytracerMT : public Raytracer
{
public:
	RaytracerMT(std::shared_ptr<std::vector<GLubyte>> imageTextureData, Camera& camera, HittableList& world, const int imageHeight, const int imageWidth, const int samplesPerPixel, const int maxDepth)
		: Raytracer(imageTextureData, camera, world, imageHeight, imageWidth, samplesPerPixel, maxDepth) {}

	virtual void run() override;

private:
	std::mutex mOutputMutex;

	void writeLines(std::vector<int> lineNumbers);
};

HittableList randomScene();
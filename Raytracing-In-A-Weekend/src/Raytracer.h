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
#include "Shader.h"

class Raytracer
{
public:
	Raytracer(std::shared_ptr<std::vector<GLubyte>> imageTextureData, Camera& camera, HittableList& world, const glm::vec3& background, const int imageHeight, const int imageWidth, const int samplesPerPixel, const int maxDepth)
		: mImageTextureData(imageTextureData), mCamera(camera), mWorld(world), mBackground(background), mImageHeight(imageHeight), mImageWidth(imageWidth), mSamplesPerPixel(samplesPerPixel), mMaxDepth(maxDepth)
	{}
	
	virtual void Run() = 0;
	virtual void Cancel() = 0;

protected:
	std::shared_ptr<std::vector<GLubyte>> mImageTextureData;
	Camera& mCamera;
	HittableList& mWorld;
	glm::vec3 mBackground;
	int mImageHeight, mImageWidth, mSamplesPerPixel, mMaxDepth;

	glm::vec3 rayColor(const Ray& r, int depth);

	void writeColor(glm::vec3 pixelColor, int samplesPerPixel, int lineNumber, int columnNumber);

};

class RaytracerNormal : public Raytracer
{
public:
	RaytracerNormal(std::shared_ptr<std::vector<GLubyte>> imageTextureData, Camera& camera, HittableList& world, const glm::vec3& background, const int imageHeight, const int imageWidth, const int samplesPerPixel, const int maxDepth)
		: Raytracer(imageTextureData, camera, world, background, imageHeight, imageWidth, samplesPerPixel, maxDepth), cancelRaytracer(false) {}

	virtual void Run() override;

	virtual void Cancel() override
	{
		cancelRaytracer = true;
	}

private:
	bool cancelRaytracer;
};

class RaytracerMT : public Raytracer
{
public:
	~RaytracerMT() = default;
	RaytracerMT(std::shared_ptr<std::vector<GLubyte>> imageTextureData, Camera& camera, HittableList& world, const glm::vec3& background, const int imageHeight, const int imageWidth, const int samplesPerPixel, const int maxDepth)
		: Raytracer(imageTextureData, camera, world, background, imageHeight, imageWidth, samplesPerPixel, maxDepth), mCurrentLineNumber(0), cancelThreads(false) {}

	virtual void Run() override;

	virtual void Cancel() override
	{
		cancelThreads = true;
	}

private:
	std::mutex mOutputMutex;
	std::mutex mLineMutex;
	int mCurrentLineNumber;
	std::vector<std::thread> threads;
	std::atomic_bool cancelThreads;

	void writeLine(int lineNumber);
};

class GPURaytracer : public Raytracer
{
public:
	GPURaytracer(std::shared_ptr<std::vector<GLubyte>> imageTextureData, Camera& camera, HittableList& world, const glm::vec3& background, const int imageHeight, const int imageWidth, const int samplesPerPixel, const int maxDepth, const int originalScreenWidth, const int originalScreenHeight);
	~GPURaytracer() = default;

	virtual void Run() override;

	virtual void Cancel()
	{
		return;
	}

private:
	std::unique_ptr<Shader> raytraceShader;
	int originalScreenWidth, originalScreenHeight;
};

HittableList randomScene();
HittableList cornellBox();
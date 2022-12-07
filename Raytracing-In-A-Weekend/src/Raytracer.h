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

struct Scene
{
	HittableList world;
	Camera camera;
	glm::vec3 background;
};

class Raytracer
{
public:
	Raytracer(std::shared_ptr<std::vector<GLubyte>> imageTextureData, Scene& renderScene, const int imageHeight, const int imageWidth, const int samplesPerPixel, const int maxDepth, const bool buildUpRender)
		: mImageTextureData(imageTextureData), mCamera(renderScene.camera), mWorld(renderScene.world), mBackground(renderScene.background), mImageHeight(imageHeight), mImageWidth(imageWidth), mSamplesPerPixel(samplesPerPixel), mMaxDepth(maxDepth), mBuildUpRender(buildUpRender), mOrigColorData(new glm::vec3[imageWidth * imageHeight])
	{
		memset(mOrigColorData, 0, sizeof(glm::vec3) * imageHeight * imageWidth);
	}
	
	virtual void Run() = 0;
	virtual void Cancel() = 0;

protected:
	std::shared_ptr<std::vector<GLubyte>> mImageTextureData;
	glm::vec3* mOrigColorData;
	Camera& mCamera;
	HittableList& mWorld;
	glm::vec3 mBackground;
	int mImageHeight, mImageWidth, mSamplesPerPixel, mMaxDepth;
	bool mBuildUpRender;

	glm::vec3 rayColor(const Ray& r, int depth);

	void writeColor(glm::vec3 pixelColor, int samplesPerPixel, int lineNumber, int columnNumber);

};

class RaytracerNormal : public Raytracer
{
public:
	RaytracerNormal(std::shared_ptr<std::vector<GLubyte>> imageTextureData, Scene& renderScene, const int imageHeight, const int imageWidth, const int samplesPerPixel, const int maxDepth, const bool buildUpRender)
		: Raytracer(imageTextureData, renderScene, imageHeight, imageWidth, samplesPerPixel, maxDepth, buildUpRender), cancelRaytracer(false) {}

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
	RaytracerMT(std::shared_ptr<std::vector<GLubyte>> imageTextureData, Scene& renderScene, const int imageHeight, const int imageWidth, const int samplesPerPixel, const int maxDepth, const bool buildUpRender)
		: Raytracer(imageTextureData, renderScene, imageHeight, imageWidth, samplesPerPixel, maxDepth, buildUpRender), mCurrentLineNumber(0), cancelThreads(false) {}

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

	void writeLine(int lineNumber, int currentSample);
};
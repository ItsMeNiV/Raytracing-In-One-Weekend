#include "Core/Raytracer.h"

glm::vec3 Raytracer::rayColor(const Ray& r, int depth)
{
	HitRecord rec;

	if (depth <= 0)
		return glm::vec3(0.0f, 0.0f, 0.0f);

	if (!mWorld.Hit(r, 0.001f, infinity, rec))
		return mBackground;

	Ray scattered;
	glm::vec3 attenuation;
	glm::vec3 emitted = rec.matPtr->emitted(rec.u, rec.v, rec.p);
	if (!rec.matPtr->scatter(r, rec, attenuation, scattered))
		return emitted;

	return emitted + attenuation * rayColor(scattered, depth - 1);
}

void Raytracer::writeColor(glm::vec3 pixelColor, int samplesPerPixel, int lineNumber, int columnNumber)
{
	auto r = pixelColor.x;
	auto g = pixelColor.y;
	auto b = pixelColor.z;

	mOrigColorData[lineNumber * mImageWidth + columnNumber] = pixelColor;

	auto scale = 1.0f / samplesPerPixel;
	r = sqrt(scale * r);
	g = sqrt(scale * g);
	b = sqrt(scale * b);

	int rValue = static_cast<int>(256 * clamp(r, 0.0f, 0.999f));
	int gValue = static_cast<int>(256 * clamp(g, 0.0f, 0.999f));
	int bValue = static_cast<int>(256 * clamp(b, 0.0f, 0.999f));

	uint32_t index = (lineNumber * mImageWidth + columnNumber) * 4;
	(*mImageTextureData)[index] = (GLubyte)rValue;
	(*mImageTextureData)[index + 1] = (GLubyte)gValue;
	(*mImageTextureData)[index + 2] = (GLubyte)bValue;
	(*mImageTextureData)[index + 3] = (GLubyte)255;
}

void RaytracerNormal::Run()
{
	if (mBuildUpRender)
	{
		for (int s = 0; s < mSamplesPerPixel; ++s)
		{
			for (int j = mImageHeight - 1; j >= 0; --j)
			{
				if (cancelRaytracer)
					return;

				for (int i = 0; i < mImageWidth; ++i)
				{
					glm::vec3 pixelColor;
					if (s == 0)
						pixelColor = glm::vec3(0.0f, 0.0f, 0.0f);
					else
						pixelColor = mOrigColorData[j * mImageWidth + i];
					float u = (i + randomFloat()) / (mImageWidth - 1);
					float v = (j + randomFloat()) / (mImageHeight - 1);
					Ray r = mCamera.GetRay(u, v);
					pixelColor += rayColor(r, mMaxDepth);
					writeColor(pixelColor, s, j, i);
				}
			}
			std::cerr << "Sample " << s << " Done." << std::endl;
		}
	}
	else
	{
		for (int j = mImageHeight - 1; j >= 0; --j)
		{
			if (cancelRaytracer)
				return;

			for (int i = 0; i < mImageWidth; ++i)
			{
				glm::vec3 pixelColor(0.0f, 0.0f, 0.0f);
				for (int s = 0; s < mSamplesPerPixel; ++s)
				{
					float u = (i + randomFloat()) / (mImageWidth - 1);
					float v = (j + randomFloat()) / (mImageHeight - 1);
					Ray r = mCamera.GetRay(u, v);
					pixelColor += rayColor(r, mMaxDepth);
				}
				writeColor(pixelColor, mSamplesPerPixel, j, i);
			}
		}
	}
}

void RaytracerMT::writeLine(int lineNumber, int currentSample)
{
	if (mBuildUpRender)
	{
		for (int i = 0; i < mImageWidth; ++i)
		{
			glm::vec3 pixelColor;
			if (currentSample == 0)
				pixelColor = glm::vec3(0.0f, 0.0f, 0.0f);
			else
				pixelColor = mOrigColorData[lineNumber * mImageWidth + i];
			float u = (i + randomFloat()) / (mImageWidth - 1);
			float v = (lineNumber + randomFloat()) / (mImageHeight - 1);
			Ray r = mCamera.GetRay(u, v);
			pixelColor += rayColor(r, mMaxDepth);
			const std::lock_guard<std::mutex> lock(mOutputMutex);
			writeColor(pixelColor, currentSample ? currentSample : 1, lineNumber, i);
		}
	}
	else
	{
		for (int i = 0; i < mImageWidth; ++i)
		{
			glm::vec3 pixelColor(0.0f, 0.0f, 0.0f);
			for (int s = 0; s < mSamplesPerPixel; ++s)
			{
				float u = (i + randomFloat()) / (mImageWidth - 1);
				float v = (lineNumber + randomFloat()) / (mImageHeight - 1);
				Ray r = mCamera.GetRay(u, v);
				pixelColor += rayColor(r, mMaxDepth);
			}
			const std::lock_guard<std::mutex> lock(mOutputMutex);
			writeColor(pixelColor, mSamplesPerPixel, lineNumber, i);
		}
	}
}

void RaytracerMT::Run()
{
	const int threadCount = std::thread::hardware_concurrency() - 2;
	int linesPerThread = mImageHeight / threadCount;

	if (mBuildUpRender)
	{
		for (int s = 0; s < mSamplesPerPixel; ++s)
		{
			mCurrentLineNumber = mImageHeight - 1;
			for (int i = 0; i < threadCount; i++)
			{
				threads.push_back(std::thread([this, s]
					{
						while (!cancelThreads)
						{
							int lineNumber = 0;
							{
								const std::lock_guard<std::mutex> lock(mLineMutex);
								lineNumber = mCurrentLineNumber--;
							}
							this->writeLine(lineNumber, s);

							if (mCurrentLineNumber == 0) return;
						}
					}));
			}

			for (std::thread& t : threads) {
				t.join();
			}
			threads.clear();
			std::cerr << "Sample " << s << " Done." << std::endl;
		}
	}
	else
	{
		mCurrentLineNumber = mImageHeight - 1;
		for (int i = 0; i < threadCount; i++)
		{
			threads.push_back(std::thread([this]
				{
					while (!cancelThreads)
					{
						int lineNumber = 0;
						{
							const std::lock_guard<std::mutex> lock(mLineMutex);
							lineNumber = mCurrentLineNumber--;
						}
						this->writeLine(lineNumber, mSamplesPerPixel);

						if (mCurrentLineNumber == 0) return;
					}
				}));
		}

		for (std::thread& t : threads) {
			t.join();
		}
	}
}
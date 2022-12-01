#include "Raytracer.h"

glm::vec3 Raytracer::rayColor(const Ray& r, int depth)
{
	HitRecord rec;

	if (depth <= 0)
		return glm::vec3(0.0f, 0.0f, 0.0f);

	if (!mWorld.Hit(r, 0.001f, infinity, rec))
		return mBackground;

	Ray scattered;
	glm::vec3 attenuation;
	glm::vec3 emitted = rec.matPtr->emitted(0.0f, 0.0f, rec.p);
	if (!rec.matPtr->scatter(r, rec, attenuation, scattered))
		return emitted;

	return emitted + attenuation * rayColor(scattered, depth - 1);
}

void Raytracer::writeColor(glm::vec3 pixelColor, int samplesPerPixel, int lineNumber, int columnNumber)
{
	auto r = pixelColor.x;
	auto g = pixelColor.y;
	auto b = pixelColor.z;

	auto scale = 1.0f / samplesPerPixel;
	r = sqrt(scale * r);
	g = sqrt(scale * g);
	b = sqrt(scale * b);

	int rValue = static_cast<int>(255.999f * clamp(r, 0.0f, 0.999f));
	int gValue = static_cast<int>(255.999f * clamp(g, 0.0f, 0.999f));
	int bValue = static_cast<int>(255.999f * clamp(b, 0.0f, 0.999f));

	uint32_t index = (lineNumber * mImageWidth + columnNumber) * 4;
	(*mImageTextureData)[index] = (GLubyte)rValue;
	(*mImageTextureData)[index + 1] = (GLubyte)gValue;
	(*mImageTextureData)[index + 2] = (GLubyte)bValue;
	(*mImageTextureData)[index + 3] = (GLubyte)255;
}

void RaytracerNormal::Run()
{
	for (int j = mImageHeight - 1; j >= 0; --j)
	{
		if (cancelRaytracer)
			return;

		std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < mImageWidth; ++i)
		{
			glm::vec3 pixelColor(0.0f, 0.0f, 0.0f);
			for (int s = 0; s < mSamplesPerPixel; ++s)
			{
				float u = (i + randomfloat()) / (mImageWidth - 1);
				float v = (j + randomfloat()) / (mImageHeight - 1);
				Ray r = mCamera.GetRay(u, v);
				pixelColor += rayColor(r, mMaxDepth);
			}
			writeColor(pixelColor, mSamplesPerPixel, j, i);
		}
	}

	std::cerr << "\nDone.\n";
}

void RaytracerMT::writeLine(int lineNumber)
{
	std::string lineString;
	for (int i = 0; i < mImageWidth; ++i)
	{
		glm::vec3 pixelColor(0.0f, 0.0f, 0.0f);
		for (int s = 0; s < mSamplesPerPixel; ++s)
		{
			float u = (i + randomfloat()) / (mImageWidth - 1);
			float v = (lineNumber + randomfloat()) / (mImageHeight - 1);
			Ray r = mCamera.GetRay(u, v);
			pixelColor += rayColor(r, mMaxDepth);
		}
		const std::lock_guard<std::mutex> lock(mOutputMutex);
		writeColor(pixelColor, mSamplesPerPixel, lineNumber, i);
	}
	std::cerr << "Line " << std::to_string(lineNumber) << " Done.\n";
}

void RaytracerMT::Run()
{
	const int threadCount = std::thread::hardware_concurrency() - 2;

	int linesPerThread = mImageHeight / threadCount;
	mCurrentLineNumber = mImageHeight-1;
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
					this->writeLine(lineNumber);

					if (mCurrentLineNumber == 0) return;
				}
		}));
	}

	for (std::thread& t : threads) {
		t.join();
	}

	std::cerr << "\nDone.\n";
}

HittableList randomScene() {
	HittableList world;

	auto groundMaterial = std::make_shared<Lambertian>(glm::vec3(0.5f, 0.5f, 0.5f));
	world.add(std::make_shared<Sphere>(glm::vec3(0.0f, -1000.0f, 0.0f), 1000.0f, groundMaterial));

	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			auto chooseMat = randomfloat();
			glm::vec3 center(a + 0.9f * randomfloat(), 0.2f, b + 0.9f * randomfloat());

			if ((center - glm::vec3(4.0f, 0.2f, 0.0f)).length() > 0.9f) {
				std::shared_ptr<Material> sphereMaterial;

				if (chooseMat < 0.8f) {
					// diffuse
					auto albedo = randomVec() * randomVec();
					sphereMaterial = std::make_shared<Lambertian>(albedo);
					world.add(std::make_shared<Sphere>(center, 0.2f, sphereMaterial));
				}
				else if (chooseMat < 0.95f) {
					// metal
					auto albedo = randomVec(0.5f, 1.0f);
					auto fuzz = randomfloat(0.0f, 0.5f);
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

	auto material1 = std::make_shared<Dielectric>(1.5f);
	world.add(make_shared<Sphere>(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f, material1));

	auto material2 = std::make_shared<Lambertian>(glm::vec3(0.4f, 0.2f, 0.1f));
	world.add(std::make_shared<Sphere>(glm::vec3(-4.0f, 1.0f, 0.0f), 1.0f, material2));

	auto material3 = std::make_shared<Metal>(glm::vec3(0.7f, 0.6f, 0.5f), 0.0f);
	world.add(std::make_shared<Sphere>(glm::vec3(4.0f, 1.0f, 0.0f), 1.0f, material3));

	return world;
}

HittableList cornellBox()
{
	HittableList objects;

	auto red = std::make_shared<Lambertian>(glm::vec3(0.65f, 0.05f, 0.05f));
	auto white = std::make_shared<Lambertian>(glm::vec3(0.73f, 0.73f, 0.73f));
	auto green = std::make_shared<Lambertian>(glm::vec3(0.12f, 0.45f, 0.15f));
	auto light = std::make_shared<DiffuseLight>(glm::vec3(15.0f, 15.0f, 15.0f));

	objects.add(make_shared<Triangle>(glm::vec3(555.0f, 0.0f, 0.0f), glm::vec3(555.0f, 555.0f, 0.0f), glm::vec3(555.0f, 555.0f, 555.0f), green, ""));
	objects.add(make_shared<Triangle>(glm::vec3(555.0f, 0.0f, 0.0f), glm::vec3(555.0f, 555.0f, 555.0f), glm::vec3(555.0f, 0.0f, 555.0f), green, "")); //Left

	objects.add(make_shared<Triangle>(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 555.0f, 0.0f), glm::vec3(0.0f, 555.0f, 555.0f), red, ""));
	objects.add(make_shared<Triangle>(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 555.0f, 555.0f), glm::vec3(0.0f, 0.0f, 555.0f), red, "")); //Right

	objects.add(make_shared<Triangle>(glm::vec3(213.0f, 554.0f, 227.0f), glm::vec3(343.0f, 554.0f, 227.0f), glm::vec3(343.0f, 554.0f, 332.0f), light, ""));
	objects.add(make_shared<Triangle>(glm::vec3(213.0f, 554.0f, 227.0f), glm::vec3(343.0f, 554.0f, 332.0f), glm::vec3(213.0f, 554.0f, 332.0f), light, "")); //Light

	objects.add(make_shared<Triangle>(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(555.0f, 0.0f, 0.0f), glm::vec3(555.0f, 0.0f, 555.0f), white, ""));
	objects.add(make_shared<Triangle>(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(555.0f, 0.0f, 555.0f), glm::vec3(0.0f, 0.0f, 555.0f), white, "")); //Floor

	objects.add(make_shared<Triangle>(glm::vec3(0.0f, 555.0f, 0.0f), glm::vec3(555.0f, 555.0f, 0.0f), glm::vec3(555.0f, 555.0f, 555.0f), white, ""));
	objects.add(make_shared<Triangle>(glm::vec3(0.0f, 555.0f, 0.0f), glm::vec3(555.0f, 555.0f, 555.0f), glm::vec3(0.0f, 555.0f, 555.0f), white, "")); //Top

	objects.add(make_shared<Triangle>(glm::vec3(0.0f, 0.0f, 555.0f), glm::vec3(555.0f, 0.0f, 555.0f), glm::vec3(555.0f, 555.0f, 555.0f), white, "back"));
	objects.add(make_shared<Triangle>(glm::vec3(0.0f, 0.0f, 555.0f), glm::vec3(555.0f, 555.0f, 555.0f), glm::vec3(0.0f, 555.0f, 555.0f), white, "back")); //Back

	return objects;
}

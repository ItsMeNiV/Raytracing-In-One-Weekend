#include "Raytracer.h"

Vec3 Raytracer::rayColor(const Ray& r, int depth)
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

void Raytracer::writeColor(Vec3 pixelColor, int samplesPerPixel)
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

void RaytracerNormal::run()
{
	std::cout << "P3\n" << mImageWidth << ' ' << mImageHeight << "\n255\n";

	for (int j = mImageHeight - 1; j >= 0; --j)
	{
		std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < mImageWidth; ++i)
		{
			Vec3 pixelColor(0.0, 0.0, 0.0);
			for (int s = 0; s < mSamplesPerPixel; ++s)
			{
				double u = (i + randomdouble()) / (mImageWidth - 1);
				double v = (j + randomdouble()) / (mImageHeight - 1);
				Ray r = mCamera.GetRay(u, v);
				pixelColor += rayColor(r, mMaxDepth);
			}
			writeColor(pixelColor, mSamplesPerPixel);
		}
	}

	std::cerr << "\nDone.\n";
}

void RaytracerMT::writeLines(std::vector<int> lineNumbers)
{
	for (int j : lineNumbers)
	{
		std::string lineString;
		for (int i = 0; i < mImageWidth; ++i)
		{
			Vec3 pixelColor(0.0, 0.0, 0.0);
			for (int s = 0; s < mSamplesPerPixel; ++s)
			{
				double u = (i + randomdouble()) / (mImageWidth - 1);
				double v = (j + randomdouble()) / (mImageHeight - 1);
				Ray r = mCamera.GetRay(u, v);
				pixelColor += rayColor(r, mMaxDepth);
			}
			auto r = pixelColor.x;
			auto g = pixelColor.y;
			auto b = pixelColor.z;

			auto scale = 1.0 / mSamplesPerPixel;
			r = sqrt(scale * r);
			g = sqrt(scale * g);
			b = sqrt(scale * b);

			uint8_t rValue = static_cast<int>(255.999 * clamp(r, 0.0, 0.999));
			uint8_t gValue = static_cast<int>(255.999 * clamp(g, 0.0, 0.999));
			uint8_t bValue = static_cast<int>(255.999 * clamp(b, 0.0, 0.999));

			const std::lock_guard<std::mutex> lock(mOutputMutex);
			(*mImageTextureData)[j][i][0] = rValue;
			(*mImageTextureData)[j][i][1] = gValue;
			(*mImageTextureData)[j][i][2] = bValue;
		}
		std::cerr << "Line " << std::to_string(j) << " Done.\n";
	}
}

void RaytracerMT::run()
{
	std::cout << "P3\n" << mImageWidth << ' ' << mImageHeight << "\n255\n";
	const int threadCount = 5;

	std::vector<std::thread> threads;
	int linesPerThread = mImageHeight / threadCount;
	int currentLine = 0;
	for (int i = 0; i < threadCount; i++)
	{
		std::vector<int> linesForThread;
		linesForThread.resize(linesPerThread);
		for (int i = 0; i < linesPerThread; i++)
		{
			linesForThread[i] = currentLine++;
		}
		threads.push_back(std::thread([this, linesForThread] { this->writeLines(linesForThread); }));
	}

	for (std::thread& t : threads) {
		t.join();
	}

	std::cerr << "\nDone.\n";
}

HittableList randomScene() {
	HittableList world;

	auto groundMaterial = std::make_shared<Lambertian>(Vec3(0.5, 0.5, 0.5));
	world.add(std::make_shared<Sphere>(Vec3(0.0, -1000.0, 0.0), 1000.0, groundMaterial));

	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			auto chooseMat = randomdouble();
			Vec3 center(a + 0.9 * randomdouble(), 0.2, b + 0.9 * randomdouble());

			if ((center - Vec3(4.0, 0.2, 0.0)).length() > 0.9) {
				std::shared_ptr<Material> sphereMaterial;

				if (chooseMat < 0.8) {
					// diffuse
					auto albedo = randomVec() * randomVec();
					sphereMaterial = std::make_shared<Lambertian>(albedo);
					world.add(std::make_shared<Sphere>(center, 0.2, sphereMaterial));
				}
				else if (chooseMat < 0.95) {
					// metal
					auto albedo = randomVec(0.5, 1.0);
					auto fuzz = randomdouble(0.0, 0.5);
					sphereMaterial = std::make_shared<Metal>(albedo, fuzz);
					world.add(std::make_shared<Sphere>(center, 0.2, sphereMaterial));
				}
				else {
					// glass
					sphereMaterial = std::make_shared<Dielectric>(1.5);
					world.add(std::make_shared<Sphere>(center, 0.2, sphereMaterial));
				}
			}
		}
	}

	auto material1 = std::make_shared<Dielectric>(1.5);
	world.add(make_shared<Sphere>(Vec3(0.0, 1.0, 0.0), 1.0, material1));

	auto material2 = std::make_shared<Lambertian>(Vec3(0.4, 0.2, 0.1));
	world.add(std::make_shared<Sphere>(Vec3(-4.0, 1.0, 0.0), 1.0, material2));

	auto material3 = std::make_shared<Metal>(Vec3(0.7, 0.6, 0.5), 0.0);
	world.add(std::make_shared<Sphere>(Vec3(4.0, 1.0, 0.0), 1.0, material3));

	return world;
}
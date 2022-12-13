#include "RaytracingApplication.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Mesh.h"
#include "Hittable.h"
#include "Bvh.h"
#include "Shader.h"
#include "ComputeShader.h"

static int imageWidth = 1600;
static int imageHeight = 900;
static int imageWidthSetting = 1600;
static int imageHeightSetting = 900;
static int samplesPerPixel = 20;
static int maxDepth = 50;
static bool useMultithreading = true;
static bool useGPUTracing = false;
static bool useBuildUpRender = true;

// timing 
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f; // time of last frame

RaytracingApplication::RaytracingApplication()
	: running(false), imageTexture(0),
	window(glfwCreateWindow(1600, 900, "Raytracing in a Weekend impl. by Yannik Hodel", NULL, NULL)), screenWidth(1600), screenHeight(900),
	imageTextureData(std::make_shared<std::vector<GLubyte>>()), renderTimeString("Time to render: 0.0s")
{
	imageTextureData->resize(imageWidth * imageHeight * 4);
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}
	glfwSetWindowSizeCallback(window, framebufferSizeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetWindowUserPointer(window, this);

	glViewport(0, 0, imageWidth, imageHeight);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");
}

RaytracingApplication::~RaytracingApplication()
{
	if(raytracerPtr)
		raytracerPtr->Cancel();
	if(raytracerThread && raytracerThread->joinable())
		raytracerThread->join();
	raytracerPtr.release();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
}

void RaytracingApplication::Run()
{
	Shader displayShader("assets/shaders/maindisplay.vert", "assets/shaders/maindisplay.frag");
	ComputeShader raytraceShader("assets/shaders/raytracer.comp");

	float fullscreenQuadVerts[] = {
		-1.0f, -1.0f, 1.0f,
		 1.0f, -1.0f, 1.0f,
		 1.0f,  1.0f, 1.0f,
		-1.0f,  1.0f, 1.0f
	};

	uint32_t fullscreenQuadIndices[] = {
		0, 1, 2,
		0, 2, 3
	};

	uint32_t vertexBuffer, indexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreenQuadVerts), fullscreenQuadVerts, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(fullscreenQuadIndices), fullscreenQuadIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenTextures(1, &imageTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, imageTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	displayShader.use();
	displayShader.setInt("ImageTexture", 0);

	int sampleCounter = 0;
	Scene gpuRaytracingScene;
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		if (raytracerThread && !running && raytracerThread->joinable())
			raytracerThread->join();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
		ImGui::Begin("Render settings", NULL, windowFlags);
		ImGui::SetWindowSize({ 400.0f, 215.0f });
		ImGui::SetWindowPos({ 0.0f, 0.0f });

		if (ImGui::Checkbox("Use GPU Raytracer", &useGPUTracing) && useGPUTracing)
		{
			sampleCounter = 0;
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, imageTexture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, imageWidth, imageHeight, 0, GL_RGBA, GL_FLOAT, NULL);
			glBindImageTexture(0, imageTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

			raytraceShader.use();
			gpuRaytracingScene = setupWorld();

			raytraceShader.setInt("imageWidth", imageWidth);
			raytraceShader.setInt("imageHeight", imageHeight);
			raytraceShader.setVec3("camera.origin", gpuRaytracingScene.camera.origin);
			raytraceShader.setVec3("camera.lowerLeftCorner", gpuRaytracingScene.camera.lowerLeftCorner);
			raytraceShader.setVec3("camera.horizontal", gpuRaytracingScene.camera.horizontal);
			raytraceShader.setVec3("camera.vertical", gpuRaytracingScene.camera.vertical);
			raytraceShader.setVec3("camera.u", gpuRaytracingScene.camera.u);
			raytraceShader.setVec3("camera.v", gpuRaytracingScene.camera.v);
			raytraceShader.setVec3("camera.w", gpuRaytracingScene.camera.w);
			raytraceShader.setFloat("camera.lensRadius", gpuRaytracingScene.camera.lensRadius);
		}
		ImGui::BeginDisabled(useGPUTracing);
		ImGui::BeginDisabled(running);
		ImGui::InputInt("Image width", &imageWidthSetting);
		ImGui::InputInt("Image height", &imageHeightSetting);
		ImGui::InputInt("Samples per pixel", &samplesPerPixel);
		ImGui::InputInt("Max depth", &maxDepth);

		ImGui::Checkbox("Use multithreading", &useMultithreading);
		ImGui::Checkbox("Use build up render", &useBuildUpRender);

		if (ImGui::Button("Render"))
		{
			if (!running)
			{
				imageWidth = imageWidthSetting;
				imageHeight = imageHeightSetting;
				running = true;
				runRaytracer();
			}
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			if (running && raytracerPtr)
			{
				raytracerPtr->Cancel();
				running = false;
			}
			if (raytracerThread && raytracerThread->joinable())
				raytracerThread->join();
		}
		ImGui::SameLine();
		ImGui::Text(renderTimeString.c_str());
		ImGui::EndDisabled();
		ImGui::End();

		displayShader.use();
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBindTexture(GL_TEXTURE_2D, imageTexture);
		if(!useGPUTracing)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageTextureData->data());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, imageTexture);

		if (useGPUTracing)
		{
			float startTime = glfwGetTime();
			raytraceShader.use();
			raytraceShader.setInt("sampleCount", sampleCounter);
			glDispatchCompute((unsigned int)imageWidth / 10, (unsigned int)imageHeight / 10, 1);

			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			renderTimeString = std::string("Frametime: " + std::to_string(glfwGetTime() - startTime) + "s");
		}

		displayShader.use();

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);

		if(useGPUTracing)
			sampleCounter++;
	}
}

void RaytracingApplication::framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	RaytracingApplication* app = (RaytracingApplication*)glfwGetWindowUserPointer(window);
	app->SetScreenDimensions(width, height);
}

void RaytracingApplication::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	RaytracingApplication* app = (RaytracingApplication*)glfwGetWindowUserPointer(window);
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		if (!app->running)
		{
			app->running = true;
			app->runRaytracer();
		}
	}
}

void RaytracingApplication::runRaytracer()
{
	imageTextureData = std::make_shared<std::vector<GLubyte>>();
	imageTextureData->resize(imageWidth * imageHeight * 4);
	raytracerThread = std::make_unique<std::thread>([this]
		{
			//Image
			const float aspectRatio = imageWidth / imageHeight;

			//World
			Scene renderScene = setupWorld();
			

			float startTime = glfwGetTime();

			//Render
			if (useMultithreading)
			{
				raytracerPtr = std::make_unique<RaytracerMT>(imageTextureData, renderScene, imageHeight, imageWidth, samplesPerPixel, maxDepth, useBuildUpRender);
			}
			else
			{
				raytracerPtr = std::make_unique<RaytracerNormal>(imageTextureData, renderScene, imageHeight, imageWidth, samplesPerPixel, maxDepth, useBuildUpRender);
			}

			raytracerPtr->Run();

			float endTime = glfwGetTime();

			renderTimeString = std::string("Time to render: " + std::to_string(endTime - startTime) + "s");
			running = false;
		});
}

Scene RaytracingApplication::setupWorld()
{
	if (useGPUTracing)
	{
		HittableList world;
		glm::vec3 background = glm::vec3(0.0f, 0.0f, 0.0f);
		const float aspectRatio = imageWidth / imageHeight;
		glm::vec3 lookfrom = { 0.0f, 0.0f, 0.0f };
		glm::vec3 lookat = { 0.0f, 0.0f, -1.0f };
		glm::vec3 vup = { 0.0f, 1.0f, 0.0f };
		float distToFocus = 10.0f;
		float aperture = 0.0f;
		Camera cam(lookfrom, lookat, vup, 40.0f, aspectRatio, aperture, distToFocus);

		return { world, cam, background };
	}
	else
	{
		HittableList world = cornellBox();// = randomScene();
		glm::vec3 background = glm::vec3(0.0f, 0.0f, 0.0f);

		glm::mat4 vaseModelMatrix(1.0f);
		vaseModelMatrix = glm::translate(vaseModelMatrix, { 277.5f, 100.00f, 277.5f });
		vaseModelMatrix = glm::scale(vaseModelMatrix, { 2000.0f, 2000.0f, 2000.0f });
		/* For Make-shift cornell box
		vaseModelMatrix = glm::translate(vaseModelMatrix, { 0.0f, 0.02f, 0.0f });
		vaseModelMatrix = glm::scale(vaseModelMatrix, {0.5f, 0.5f, 0.5f});
		*/
		world.add(std::make_shared<Mesh>(vaseModelMatrix, "assets/models/brass_vase/brass_vase_04_4k.gltf"));

		/*
		auto white = std::make_shared<Lambertian>(glm::vec3(0.73f, 0.73f, 0.73f));

		glm::mat4 box1Model(1.0f);
		box1Model = glm::translate(box1Model, glm::vec3(265.0f, 0.0f, 295.0f));
		box1Model = glm::rotate(box1Model, glm::radians(15.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		world.add(std::make_shared<Box>(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(165.0f, 330.0f, 165.0f), white, box1Model));

		glm::mat4 box2Model(1.0f);
		box2Model = glm::translate(box2Model, glm::vec3(130.0f, 0.0f, 65.0f));
		box2Model = glm::rotate(box2Model, glm::radians(-18.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		world.add(std::make_shared<Box>(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(165.0f, 165.0f, 165.0f), white, box2Model));
		*/

		/* Make-shift cornell box
		auto greenMat = std::make_shared<Lambertian>(glm::vec3(0.0f, 1.0f, 0.0f));
		auto redMat = std::make_shared<Lambertian>(glm::vec3(1.0f, 0.0f, 0.0f));
		auto blueMat = std::make_shared<Lambertian>(glm::vec3(0.0f, 0.0f, 1.0f));
		auto groundMaterial = std::make_shared<Lambertian>(glm::vec3(0.5f, 0.5f, 0.5f));
		auto lightMat = std::make_shared<DiffuseLight>(glm::vec3(1.0f, 1.0f, 1.0f));
		world.add(std::make_shared<Sphere>(glm::vec3(1.1f, 0.0f, 0.0f), 1.0f, greenMat));
		world.add(std::make_shared<Sphere>(glm::vec3(-1.1f, 0.0f, 0.0f), 1.0f, redMat));
		world.add(std::make_shared<Sphere>(glm::vec3(0.0f, 0.0f, 1.1f), 1.0f, blueMat));
		world.add(std::make_shared<Sphere>(glm::vec3(0.0f, 1.3f, 0.0f), 1.1f, lightMat));
		world.add(std::make_shared<Sphere>(glm::vec3(0.0f, -1000.0f, 0.0f), 1000.0f, groundMaterial));
		*/

		//Camera
		const float aspectRatio = imageWidth / imageHeight;
		glm::vec3 lookfrom = { 278.0f, 278.0f, -800.0f };
		//glm::vec3 lookfrom = { 0.0f, 0.1f, -0.5f };
		//glm::vec3 lookat = { 0.0f, 0.1f, 0.0f };
		glm::vec3 lookat = { 278.0f, 278.0f, 0.0f };
		glm::vec3 vup = { 0.0f, 1.0f, 0.0f };
		float distToFocus = 10.0f;
		float aperture = 0.0f;
		Camera cam(lookfrom, lookat, vup, 40.0f, aspectRatio, aperture, distToFocus);

		return { world, cam, background };
	}
}

int main()
{
	if (!glfwInit())
		return -1;

	RaytracingApplication app;
	app.Run();
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

	Vertex vert0;
	Vertex vert1;
	Vertex vert2;
	vert0.normal = { 0.0f, 0.0f, 0.0f };
	vert1.normal = { 0.0f, 0.0f, 0.0f };
	vert2.normal = { 0.0f, 0.0f, 0.0f };

	vert0.position = glm::vec3(555.0f, 0.0f, 0.0f);
	vert1.position = glm::vec3(555.0f, 555.0f, 0.0f);
	vert2.position = glm::vec3(555.0f, 555.0f, 555.0f);
	objects.add(make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), green, ""));
	vert0.position = glm::vec3(555.0f, 0.0f, 0.0f);
	vert1.position = glm::vec3(555.0f, 555.0f, 555.0f);
	vert2.position = glm::vec3(555.0f, 0.0f, 555.0f);
	objects.add(make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), green, "")); //Left

	vert0.position = glm::vec3(0.0f, 0.0f, 0.0f);
	vert1.position = glm::vec3(0.0f, 555.0f, 0.0f);
	vert2.position = glm::vec3(0.0f, 555.0f, 555.0f);
	objects.add(make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), red, ""));
	vert0.position = glm::vec3(0.0f, 0.0f, 0.0f);
	vert1.position = glm::vec3(0.0f, 555.0f, 555.0f);
	vert2.position = glm::vec3(0.0f, 0.0f, 555.0f);
	objects.add(make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), red, "")); //Right

	vert0.position = glm::vec3(213.0f, 554.0f, 227.0f);
	vert1.position = glm::vec3(343.0f, 554.0f, 227.0f);
	vert2.position = glm::vec3(343.0f, 554.0f, 332.0f);
	objects.add(make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), light, ""));
	vert0.position = glm::vec3(213.0f, 554.0f, 227.0f);
	vert1.position = glm::vec3(343.0f, 554.0f, 332.0f);
	vert2.position = glm::vec3(213.0f, 554.0f, 332.0f);
	objects.add(make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), light, "")); //Light

	vert0.position = glm::vec3(0.0f, 0.0f, 0.0f);
	vert1.position = glm::vec3(555.0f, 0.0f, 0.0f);
	vert2.position = glm::vec3(555.0f, 0.0f, 555.0f);
	objects.add(make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), white, ""));
	vert0.position = glm::vec3(0.0f, 0.0f, 0.0f);
	vert1.position = glm::vec3(555.0f, 0.0f, 555.0f);
	vert2.position = glm::vec3(0.0f, 0.0f, 555.0f);
	objects.add(make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), white, "")); //Floor

	vert0.position = glm::vec3(0.0f, 555.0f, 0.0f);
	vert1.position = glm::vec3(555.0f, 555.0f, 0.0f);
	vert2.position = glm::vec3(555.0f, 555.0f, 555.0f);
	objects.add(make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), white, ""));
	vert0.position = glm::vec3(0.0f, 555.0f, 0.0f);
	vert1.position = glm::vec3(555.0f, 555.0f, 555.0f);
	vert2.position = glm::vec3(0.0f, 555.0f, 555.0f);
	objects.add(make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), white, "")); //Top

	vert0.position = glm::vec3(0.0f, 0.0f, 555.0f);
	vert1.position = glm::vec3(555.0f, 0.0f, 555.0f);
	vert2.position = glm::vec3(555.0f, 555.0f, 555.0f);
	objects.add(make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), white, "back"));
	vert0.position = glm::vec3(0.0f, 0.0f, 555.0f);
	vert1.position = glm::vec3(555.0f, 555.0f, 555.0f);
	vert2.position = glm::vec3(0.0f, 555.0f, 555.0f);
	objects.add(make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), white, "back")); //Back

	return objects;
}

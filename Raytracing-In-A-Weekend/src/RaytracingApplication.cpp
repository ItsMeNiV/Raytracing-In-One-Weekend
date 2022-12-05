#include "RaytracingApplication.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Mesh.h"
#include "Hittable.h"

static int imageWidth = 1600;
static int imageHeight = 900;
static int imageWidthSetting = 1600;
static int imageHeightSetting = 900;
static int samplesPerPixel = 20;
static int maxDepth = 50;
static bool useMultithreading = true;
static bool useGPUTracing = false;
static bool useBuildUpRender = true;

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
	glBindTexture(GL_TEXTURE_2D, imageTexture);

	while (!glfwWindowShouldClose(window))
	{
		displayShader.use();
		glfwPollEvents();

		if (raytracerThread && !running && raytracerThread->joinable())
			raytracerThread->join();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
		ImGui::Begin("Render settings", NULL, windowFlags);
		ImGui::SetWindowSize({ 400.0f, 195.0f });
		ImGui::SetWindowPos({ 0.0f, 0.0f });

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
		ImGui::End();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindTexture(GL_TEXTURE_2D, imageTexture);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageTextureData->data());
		glGenerateMipmap(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, imageTexture);
		displayShader.setInt("ImageTexture", 0);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
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
	//Image
	const float aspectRatio = imageWidth / imageHeight;

	//World
	HittableList world = randomScene();
	glm::vec3 background(1.0f, 1.0f, 1.0f);

	//Camera
	glm::vec3 lookfrom = { 13.0f, 2.0f, 3.0f };
	glm::vec3 lookat = { 0.0f, 0.0f, 0.0f };
	glm::vec3 vup = { 0.0f, 1.0f, 0.0f };
	float distToFocus = 10.0f;
	float aperture = 0.1f;
	Camera cam(lookfrom, lookat, vup, 20.0f, aspectRatio, aperture, distToFocus);

	imageTextureData = std::make_shared<std::vector<GLubyte>>();
	imageTextureData->resize(imageWidth * imageHeight * 4);
	if (useGPUTracing)
	{
		raytracerPtr = std::make_unique<GPURaytracer>(imageTextureData, cam, world, background, imageHeight, imageWidth, samplesPerPixel, maxDepth, screenWidth, screenHeight);
		raytracerPtr->Run();
		running = false;
	}
	else
	{
		raytracerThread = std::make_unique<std::thread>([this]
			{
				//Image
				const float aspectRatio = imageWidth / imageHeight;

				//World
				HittableList world;// = cornellBox();// = randomScene();
				glm::vec3 background = glm::vec3(1.0f, 1.0f, 1.0f);

				glm::mat4 vaseModelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
				auto vase = std::make_shared<Mesh>(vaseModelMatrix, "assets/models/brass_vase/brass_vase_04_4k.gltf");
				world.add(vase);

				auto greenMat = std::make_shared<Lambertian>(glm::vec3(0.0f, 1.0f, 0.0f));
				auto redMat = std::make_shared<Lambertian>(glm::vec3(1.0f, 0.0f, 0.0f));
				auto blueMat = std::make_shared<Lambertian>(glm::vec3(0.0f, 0.0f, 1.0f));
				auto groundMaterial = std::make_shared<Lambertian>(glm::vec3(0.5f, 0.5f, 0.5f));
				auto lightMat = std::make_shared<DiffuseLight>(glm::vec3(1.0f, 1.0f, 1.0f));
				world.add(std::make_shared<Sphere>(glm::vec3(1.1f, 0.0f, 0.0f), 1.0f, greenMat));
				world.add(std::make_shared<Sphere>(glm::vec3(-1.1f, 0.0f, 0.0f), 1.0f, redMat));
				world.add(std::make_shared<Sphere>(glm::vec3(0.0f, 0.0f, 1.1f), 1.0f, blueMat));
				world.add(std::make_shared<Sphere>(glm::vec3(0.0f, 1.3f, 0.0f), 1.0f, lightMat));
				world.add(std::make_shared<Sphere>(glm::vec3(0.0f, -1000.0f, 0.0f), 1000.0f, groundMaterial));

				//Camera
				//glm::vec3 lookfrom = { 278f.0, 278f.0, -800f.0 };
				glm::vec3 lookfrom = { 0.0f, 0.1f, -0.5f };
				glm::vec3 lookat = { 0.0f, 0.1f, 0.0f };
				glm::vec3 vup = { 0.0f, 1.0f, 0.0f };
				float distToFocus = 10.0f;
				float aperture = 0.0f;
				Camera cam(lookfrom, lookat, vup, 40.0f, aspectRatio, aperture, distToFocus);

				float startTime = glfwGetTime();

				//Render
				if (useMultithreading)
				{
					raytracerPtr = std::make_unique<RaytracerMT>(imageTextureData, cam, world, background, imageHeight, imageWidth, samplesPerPixel, maxDepth, useBuildUpRender);
				}
				else
				{
					raytracerPtr = std::make_unique<RaytracerNormal>(imageTextureData, cam, world, background, imageHeight, imageWidth, samplesPerPixel, maxDepth, useBuildUpRender);
				}

				raytracerPtr->Run();

				float endTime = glfwGetTime();

				renderTimeString = std::string("Time to render: " + std::to_string(endTime - startTime) + "s");
				running = false;
			});
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

	/*
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
	*/

	return objects;
}

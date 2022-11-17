#include "RaytracingApplication.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

static int imageWidth = 1600;
static int imageHeight = 900;
static int samplesPerPixel = 20;
static int maxDepth = 50;
static bool useMultithreading = true;
static bool useGPUTracing = false;

RaytracingApplication::RaytracingApplication()
	: running(false), imageTexture(0),
	window(glfwCreateWindow(1600, 900, "Raytracing in a Weekend impl. by Yannik Hodel", NULL, NULL)), screenWidth(1600), screenHeight(900),
	imageTextureData(std::make_shared<std::vector<GLubyte>>())
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
		ImGui::SetWindowSize({ 400.0f, 170.0f });
		ImGui::SetWindowPos({ 0.0f, 0.0f });

		ImGui::BeginDisabled(running);
		ImGui::InputInt("Image width", &imageWidth);
		ImGui::InputInt("Image height", &imageHeight);
		ImGui::InputInt("Samples per pixel", &samplesPerPixel);
		ImGui::InputInt("Max depth", &maxDepth);

		ImGui::Checkbox("Use multithreading", &useMultithreading);

		if (ImGui::Button("Render"))
		{
			if (!running)
			{
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
	const double aspectRatio = imageWidth / imageHeight;

	//World
	HittableList world = randomScene();
	Vec3 background(1.0, 1.0, 1.0);

	//Camera
	Vec3 lookfrom = { 13.0, 2.0, 3.0 };
	Vec3 lookat = { 0.0, 0.0, 0.0 };
	Vec3 vup = { 0.0, 1.0, 0.0 };
	double distToFocus = 10.0;
	double aperture = 0.1;
	Camera cam(lookfrom, lookat, vup, 20.0, aspectRatio, aperture, distToFocus);

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
				const double aspectRatio = imageWidth / imageHeight;

				//World
				HittableList world = cornellBox();// = randomScene();
				Vec3 background = Vec3(0.0, 0.0, 0.0);

				//Camera
				Vec3 lookfrom = { 278.0, 278.0, -800.0 };
				Vec3 lookat = { 278.0, 278.0, 0.0 };
				Vec3 vup = { 0.0, 1.0, 0.0 };
				double distToFocus = 10.0;
				double aperture = 0.1;
				Camera cam(lookfrom, lookat, vup, 40.0, aspectRatio, aperture, distToFocus);

				//Render
				if (useMultithreading)
				{
					raytracerPtr = std::make_unique<RaytracerMT>(imageTextureData, cam, world, background, imageHeight, imageWidth, samplesPerPixel, maxDepth);
				}
				else
				{
					raytracerPtr = std::make_unique<RaytracerNormal>(imageTextureData, cam, world, background, imageHeight, imageWidth, samplesPerPixel, maxDepth);
				}

				raytracerPtr->Run();


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
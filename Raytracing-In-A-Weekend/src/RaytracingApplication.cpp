#include "RaytracingApplication.h"
#include <array>
#include "Raytracer.h"

RaytracingApplication::RaytracingApplication()
	: window(glfwCreateWindow(800, 600, "Raytracing in a Weekend impl. by Yannik Hodel", NULL, NULL)), running(false),
	imageTextureData(std::make_shared<std::array<std::array<std::array<GLubyte, 3>, 800>, 600>>())
{
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}
	glfwSetWindowSizeCallback(window, framebufferSizeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetWindowUserPointer(window, this);

	glViewport(0, 0, 800, 600);
}

RaytracingApplication::~RaytracingApplication()
{
	glfwTerminate();
}

void RaytracingApplication::Run()
{
	const char* vertexShaderSource = "#version 460 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"layout (location = 1) in vec2 aTexCoord;\n"
		"out vec2 TexCoord;\n"
		"void main()\n"
		"{\n"
		"   TexCoord = aTexCoord;\n"
		"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		"}\0";
	const char* fragmentShaderSource = "#version 460 core\n"
		"out vec4 FragColor;\n"
		"in vec2 TexCoord;\n"
		"uniform sampler2D ImageTexture;\n"
		"void main()\n"
		"{\n"
		"   FragColor = texture(ImageTexture, TexCoord);\n"
		"}\0";
	uint32_t vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	int  success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	uint32_t fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	uint32_t shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glUseProgram(shaderProgram);

	float fullscreenQuadVerts[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f
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

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glGenTextures(1, &imageTexture);
	glBindTexture(GL_TEXTURE_2D, imageTexture);

	while (!glfwWindowShouldClose(window))
	{
		if (rayTracerThread && !running && rayTracerThread->joinable())
			rayTracerThread->join();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, imageTextureData.get());
		glGenerateMipmap(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, imageTexture);
		glUniform1i(glGetUniformLocation(shaderProgram, "ImageTexture"), 0);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void RaytracingApplication::framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
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
	imageTextureData = std::make_shared<std::array<std::array<std::array<GLubyte, 3>, 800>, 600>>();
	rayTracerThread = std::make_unique<std::thread>([this]
	{
			//Image
			const double aspectRatio = 4.0 / 3.0;
			const int imageWidth = 800;
			const int imageHeight = static_cast<int>(imageWidth / aspectRatio);
			const int samplesPerPixel = 2;
			const int maxDepth = 5;

			//World
			HittableList world = randomScene();

			//Camera
			Vec3 lookfrom = { 13.0, 2.0, 3.0 };
			Vec3 lookat = { 0.0, 0.0, 0.0 };
			Vec3 vup = { 0.0, 1.0, 0.0 };
			double distToFocus = 10.0;
			double aperture = 0.1;
			Camera cam(lookfrom, lookat, vup, 20.0, aspectRatio, aperture, distToFocus);

			//Render
			//RaytracerNormal tracer(fout, cam, world, imageHeight, imageWidth, samplesPerPixel, maxDepth);
			RaytracerMT tracer(imageTextureData, cam, world, imageHeight, imageWidth, samplesPerPixel, maxDepth);
			tracer.run();
			running = false;
	});
}

int main()
{
	if (!glfwInit())
		return -1;

	RaytracingApplication app;
	app.Run();
}
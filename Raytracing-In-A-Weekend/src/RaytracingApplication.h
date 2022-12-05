#pragma once
#include <thread>
#include <memory>
#include <vector>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Raytracer.h"
#include "Shader.h"

class RaytracingApplication
{
public:
    RaytracingApplication();
    ~RaytracingApplication();

    void Run();
    void runRaytracer();

    void SetScreenDimensions(int32_t width, int32_t height)
    {
        screenWidth = width;
        screenHeight = height;
    }

private:
    GLFWwindow* window;
    bool running;
    std::unique_ptr<std::thread> raytracerThread;
    std::unique_ptr<Raytracer> raytracerPtr;
    uint32_t imageTexture;
    int32_t screenWidth, screenHeight;
    std::shared_ptr<std::vector<GLubyte>> imageTextureData;
    std::string renderTimeString;

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};

HittableList randomScene();
HittableList cornellBox();
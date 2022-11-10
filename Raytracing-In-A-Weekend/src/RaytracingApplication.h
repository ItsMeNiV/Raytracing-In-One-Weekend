#pragma once
#include <thread>
#include <memory>
#include <vector>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Raytracer.h"

class RaytracingApplication
{
public:
    RaytracingApplication();
    ~RaytracingApplication();

    void Run();
    void runRaytracer();

private:
    GLFWwindow* window;
    bool running;
    std::unique_ptr<std::thread> raytracerThread;
    std::unique_ptr<RaytracerMT> raytracerPtr;
    uint32_t imageTexture;
    std::shared_ptr<std::vector<GLubyte>> imageTextureData;

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
# CMakeList.txt: CMake-Projekt für "Raytracing-In-A-Weekend". Schließen Sie die Quelle ein, und definieren Sie
# projektspezifische Logik hier.
#
cmake_minimum_required (VERSION 3.8)

# Fügen Sie der ausführbaren Datei dieses Projekts eine Quelle hinzu.
add_executable (${CMAKE_PROJECT_NAME} "vendor/glad/src/glad.c" "src/Raytracer.cpp" "src/Vec3.h" "src/Vec3.cpp" "src/Ray.h" "src/Hittable.h" "src/RTWeekend.h" "src/RTWeekend.cpp" "src/Camera.h" "src/Material.h"  "src/Raytracer.h"   "src/RaytracingApplication.cpp")

add_subdirectory("vendor/glfw")

include_directories(${CMAKE_PROJECT_NAME} "vendor/GLFW/include" "vendor/glad/include" "vendor/glm")
target_link_libraries(${CMAKE_PROJECT_NAME} glfw)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()

# TODO: Fügen Sie bei Bedarf Tests hinzu, und installieren Sie Ziele.

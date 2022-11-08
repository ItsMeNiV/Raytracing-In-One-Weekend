# CMakeList.txt: CMake-Projekt für "Raytracing-In-A-Weekend". Schließen Sie die Quelle ein, und definieren Sie
# projektspezifische Logik hier.
#
cmake_minimum_required (VERSION 3.8)

# Fügen Sie der ausführbaren Datei dieses Projekts eine Quelle hinzu.
add_executable (Raytracing-In-A-Weekend "src/Raytracer.cpp" "src/Vec3.h" "src/Ray.h" "src/Hittable.h" "src/RTWeekend.h" "src/Camera.h" "src/Material.h"  "src/Raytracer.h"  )

target_include_directories(Raytracing-In-A-Weekend PRIVATE "vendor/glm")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Raytracing-In-A-Weekend PROPERTY CXX_STANDARD 20)
endif()

# TODO: Fügen Sie bei Bedarf Tests hinzu, und installieren Sie Ziele.

# CMakeList.txt: CMake-Projekt für "Raytracing-In-A-Weekend". Schließen Sie die Quelle ein, und definieren Sie
# projektspezifische Logik hier.
#
cmake_minimum_required (VERSION 3.8)

set(IMGUI_FILES
		"vendor/imgui/imconfig.h"
		"vendor/imgui/imgui.h"
		"vendor/imgui/imgui.cpp"
		"vendor/imgui/imgui_draw.cpp"
		"vendor/imgui/imgui_internal.h"
		"vendor/imgui/imgui_tables.cpp"
		"vendor/imgui/imgui_widgets.cpp"
		"vendor/imgui/imstb_rectpack.h"
		"vendor/imgui/imstb_textedit.h"
		"vendor/imgui/imstb_truetype.h"
		"vendor/imgui/imgui_demo.cpp"
		"vendor/imgui/backends/imgui_impl_glfw.cpp"
		"vendor/imgui/backends/imgui_impl_glfw.h"
		"vendor/imgui/backends/imgui_impl_opengl3.cpp"
		"vendor/imgui/backends/imgui_impl_opengl3.h"
		"vendor/imgui/backends/imgui_impl_opengl3_loader.h"
)

set(LIB_FILES
	"vendor/glad/src/glad.c"
	"vendor/stb_image/stb_image.cpp"
)

set(PRJ_SRC_FILES
	"src/RaytracingApplication.h"
	"src/RaytracingApplication.cpp"
	"src/Core/Raytracer.h"
	"src/Core/Raytracer.cpp"
	"src/Core/Ray.h"
	"src/Core/Hittable.h"
	"src/Core/RTWeekend.h"
	"src/Core/RTWeekend.cpp"
	"src/Core/Camera.h"
	"src/Core/Mesh.h"
	"src/Core/Mesh.cpp" 
	"src/Material/Material.h"
	"src/Material/Texture.h"
	"src/AccelerationStructures/Bvh.h"
	"src/AccelerationStructures/Bvh.cpp"
	"src/AccelerationStructures/AABB.h"
	"src/AccelerationStructures/AABB.cpp"
	"src/Shader/Shader.h"
	"src/Shader/ComputeShader.h"
)

set(INCLUDE_DIRS
	"src"
	"vendor/glfw/include"
	"vendor/glad/include"
	"vendor/glm"
	"vendor/imgui"
	"vendor/imgui/backends"
	"vendor/assimp/include"
	"vendor/stb_image"
)

# Fügen Sie der ausführbaren Datei dieses Projekts eine Quelle hinzu.
add_executable (${CMAKE_PROJECT_NAME} ${IMGUI_FILES} ${LIB_FILES} ${PRJ_SRC_FILES})

add_subdirectory("vendor/glfw")
add_subdirectory("vendor/assimp")

include_directories(${CMAKE_PROJECT_NAME} ${INCLUDE_DIRS})
target_link_libraries(${CMAKE_PROJECT_NAME} glfw assimp)

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
)
add_dependencies(${CMAKE_PROJECT_NAME} copy_assets)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()

# TODO: Fügen Sie bei Bedarf Tests hinzu, und installieren Sie Ziele.

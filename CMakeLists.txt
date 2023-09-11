cmake_minimum_required(VERSION 3.5)

project(Vulkan_renderer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

option(ASSIMP_BUILD_TESTS OFF)
option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
#option(VMA_BUILD_SAMPLES OFF)

find_package(Vulkan REQUIRED)
add_subdirectory(ext/glm)
add_subdirectory(ext/glfw-3.3.8)
add_subdirectory(ext/assimp-5.2.5)
add_subdirectory(ext/VulkanMemoryAllocator)
include_directories(ext/stb-master)

file(GLOB_RECURSE CPP_SOURCE_FILES "src/*.h" "src/*.cpp")
add_executable(PBR ${CPP_SOURCE_FILES})

target_link_libraries(PBR PUBLIC glm PUBLIC glfw PUBLIC assimp PUBLIC VulkanMemoryAllocator Vulkan::Vulkan)

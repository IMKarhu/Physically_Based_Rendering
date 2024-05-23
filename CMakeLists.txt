cmake_minimum_required(VERSION 3.5)

project(Vulkan_renderer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

find_package(Vulkan REQUIRED)
add_subdirectory(ext/glm)
add_subdirectory(ext/glfw-3.3.8)
add_subdirectory(ext/VulkanMemoryAllocator)


file(GLOB_RECURSE CPP_SOURCE_FILES "src/*.h" "src/*.cpp")
add_executable(PBR ${CPP_SOURCE_FILES})

target_link_libraries(PBR PUBLIC glm PUBLIC glfw PUBLIC VulkanMemoryAllocator Vulkan::Vulkan)

cmake_minimum_required(VERSION 3.20)

project(Vulkan_renderer)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)
add_subdirectory(ext/glm)
add_subdirectory(ext/glfw-3.3.8)
add_subdirectory(ext/VulkanMemoryAllocator)
add_subdirectory(ext/assimp-5.4.3)

include_directories("ext/stb-master")


file(GLOB_RECURSE CPP_SOURCE_FILES "src/*.h" "src/*.hpp" "src/*.cpp")
add_executable(PBR ${CPP_SOURCE_FILES})

target_link_libraries(PBR PUBLIC glm PUBLIC glfw PUBLIC VulkanMemoryAllocator PUBLIC assimp Vulkan::Vulkan)

message(CFLAGS=${COMPILE_FLAGS})

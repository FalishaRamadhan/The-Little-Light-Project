cmake_minimum_required(VERSION 3.10)
project(little_light CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Interactive build: OpenGL + GLFW window (CP3)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
add_executable(little_light main.cpp)
target_link_libraries(little_light PRIVATE glfw OpenGL::GL)

# Pure C++ build (CP1/CP2): no OpenGL, no libraries. Renders frames / video to disk.
add_executable(little_light_software main.cpp)
target_compile_definitions(little_light_software PRIVATE NO_WINDOW)

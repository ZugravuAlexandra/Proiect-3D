cmake_minimum_required(VERSION 3.30)
project(proiect_3d)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(proiect_3d main.cpp src/loadShaders.cpp src/objloader.cpp
        Camera.h
        Particle.h
)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)

target_link_libraries(proiect_3d OpenGL::GL GLUT::GLUT GLEW::GLEW GLU SOIL)
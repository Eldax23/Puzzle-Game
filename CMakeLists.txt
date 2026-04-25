cmake_minimum_required(VERSION 3.10)
project(mathgrid)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(GLUT   REQUIRED)

add_executable(mathgrid main.cpp)

target_include_directories(mathgrid PRIVATE ${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})
target_link_libraries(mathgrid ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})

cmake_minimum_required(VERSION 3.12)
project(GravitySimulatorVTK)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(gravity_sim src/main.cpp)

target_link_libraries(gravity_sim PRIVATE ${VTK_LIBRARIES})
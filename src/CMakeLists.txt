cmake_minimum_required(VERSION 3.13)

file(GLOB projectSRC "*/*.h")

add_library(libmydisas INTERFACE)

message(STATUS "CMake inatall directory: " ${CMAKE_INSTALL_INCLUDEDIR})

target_include_directories(libmydisas
    INTERFACE 
        $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

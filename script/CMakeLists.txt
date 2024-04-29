cmake_minimum_required(VERSION 3.13)

add_executable(mydisas main.cpp)

target_link_libraries(mydisas pthread libmydisas)

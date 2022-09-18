cmake_minimum_required(VERSION 3.22)
project(outfittable C)

set(CMAKE_FIND_DEBUG_MODE 1)
set(CMAKE_C_STANDARD 99)

find_package(CURL CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(json-c CONFIG REQUIRED)

add_executable(outfittable main.c auth.h auth.c fit.c fit.h)
target_link_libraries(outfittable PRIVATE SDL2::SDL2 SDL2::SDL2main CURL::libcurl json-c::json-c)

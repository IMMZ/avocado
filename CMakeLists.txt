project(avocado)
cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 20 CACHE STRING "cpp standard")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE STRING "cpp standard required")
set(CMAKE_CXX_EXTENSIONS OFF CACHE STRING "cpp standard extensions")

add_subdirectory(avocadoeng)
add_subdirectory(game)


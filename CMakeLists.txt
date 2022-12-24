cmake_minimum_required(VERSION 3.17)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(PROJECT_NAME Roc)
set(EXECUTABLE_NAME roc)

project("${PROJECT_NAME}" VERSION 0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

add_subdirectory(include)
add_subdirectory(src)

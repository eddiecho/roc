cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(PROJECT_NAME Roc)
set(EXECUTABLE_NAME roc)

project("${PROJECT_NAME}" VERSION 0.1)

add_subdirectory(include)
add_subdirectory(src)


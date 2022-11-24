cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(PROJECT_NAME Roc)
set(EXECUTABLE_NAME roc)

project("${PROJECT_NAME}" VERSION 0.1)
configure_file(src/roc_config.h.in src/roc_config.h)

add_executable("${EXECUTABLE_NAME}" src/main.cpp)
target_include_directories("${EXECUTABLE_NAME}" PUBLIC
  "${PROJECT_BINARY_DIR}/src"
  "${PROJECT_SOURCE_DIR}/src")

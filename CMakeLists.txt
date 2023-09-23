cmake_minimum_required(VERSION 3.17)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(PROJECT_NAME Roc)
set(EXECUTABLE_NAME roc)

project("${PROJECT_NAME}" VERSION 0.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

# set(PROFILER_FLAGS "-lprofiler")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${PROFILER_FLAGS}")

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY "https://github.com/google/googletest.git"
  GIT_TAG        "release-1.12.1"
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  absl
  GIT_REPOSITORY "https://github.com/abseil/abseil-cpp.git"
  GIT_TAG        "20230802.1"
)
FetchContent_MakeAvailable(absl)

set(THIRD_PARTY_LIB
  "absl::hash"
  "absl::flat_hash_map"
)

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(test)

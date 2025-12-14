cmake_minimum_required(VERSION 3.17)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS off)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(PROJECT_NAME Roc)
set(EXE_NAME roc)
project("${PROJECT_NAME}" VERSION 0.1)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE
      Debug
      CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                                               "MinSizeRel" "RelWithDebInfo")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# set(PROFILER_FLAGS "-lprofiler") set(CMAKE_EXE_LINKER_FLAGS
# "${CMAKE_EXE_LINKER_FLAGS} ${PROFILER_FLAGS}")

function(set_compiler_flags target)
  if(MSVC)
    target_compile_options(
      ${target}
      PRIVATE /EHs-c-
              /D_HAS_EXCEPTIONS=0
              /W4 # Warning level 4 (highest reasonable level)
              /w14456 # Shadow: local variable
              /w14457 # Shadow: parameter
              /w14458 # Shadow: member
              /w14459 # Shadow: global
              /w14242 # Conversion warnings
              /w14668 # Undefined in preprocessor
              /w14716 # Must return a value
              /w14906 # String literal cast
              /w14061 # Switch enum not handled
              /w14100 # Unreferenced formal parameter
              $<$<CONFIG:Debug>:/Od
              /Zi
              /MDd>
              $<$<CONFIG:Release>:/O2
              /MD
              /DNDEBUG>
              $<$<CONFIG:RelWithDebInfo>:/O2
              /Zi
              /MD>
              $<$<CONFIG:MinSizeRel>:/O1
              /DNDEBUG
              /MD>)
  else()
    target_compile_options(
      ${target}
      PRIVATE -fno-exceptions
              -Wall
              -Wshadow
              -Wconversion
              -Wundef
              -Wextra
              -Wpointer-arith
              -Wreturn-type
              -Wcast-qual
              -Wwrite-strings
              -Wswitch
              -Wunused-parameter
              -Wcast-align
              -Winline
              -Wredundant-decls
              -Wchar-subscripts
              $<$<CONFIG:Debug>:-g
              -O0
              -DDEBUG>
              $<$<CONFIG:Release>:-O3
              -DNDEBUG>
              $<$<CONFIG:RelWithDebInfo>:-O2
              -g
              -DDEBUG>
              $<$<CONFIG:MinSizeRel>:-Os
              -DNDEBUG>)
  endif()
endfunction()

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY "https://github.com/google/googletest.git"
  GIT_TAG "v1.17.0")
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_subdirectory(src)
add_subdirectory(test)

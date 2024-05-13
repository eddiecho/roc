file(GLOB_RECURSE HDRFILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE SRCFILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cc")

source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header"
  FILES ${HDRFILES}
)

add_executable("${EXECUTABLE_NAME}" ${SRCFILES})

if (MSVC)
target_compile_options("${EXECUTABLE_NAME}" PRIVATE
  "/W4"
#  "/WX" warnings as errors
  "/kernel" # no exceptions, no rtti, no new, delete operators by default
  "/MD" # statically link C runtime
)
else()
target_compile_options("${EXECUTABLE_NAME}" PRIVATE
  "-Wall"
  "-Wextra"
  "-Wshadow"
  "-Wconversion"
  "-Wpedantic"
  "-fno-exceptions"
  "-static"
# "-Werror" warnings as errors
)
endif()

set(CMAKE_UNITY_BUILD TRUE)

target_include_directories("${EXECUTABLE_NAME}" PUBLIC
  "${PROJECT_BINARY_DIR}/include"
  "${PROJECT_SOURCE_DIR}/include")

target_link_libraries(
  "${EXECUTABLE_NAME}"
  ${THIRD_PARTY_LIB}
)

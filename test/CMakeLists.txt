enable_testing()

set(TEST_EXE_NAME roc_test)

file(GLOB_RECURSE TSTFILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/test/*.cc")

file(GLOB_RECURSE TSTSRCS CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cc")
get_filename_component(main_file ../src/main.cc ABSOLUTE)
list(REMOVE_ITEM TSTSRCS "${main_file}")

add_executable(
  "${TEST_EXE_NAME}"
  ${TSTFILES}
  ${TSTSRCS}
)

if (MSVC)
target_compile_options("${TEST_EXE_NAME}" PRIVATE
  "/W4"
#  "/WX" warnings as errors
  "/kernel" # no exceptions, no rtti, no new, delete operators by default
  "/MD" # statically link C runtime
)
else()
target_compile_options("${TEST_EXE_NAME}" PRIVATE
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

target_include_directories(
  "${TEST_EXE_NAME}"
  PUBLIC
  "${PROJECT_BINARY_DIR}/include"
  "${PROJECT_SOURCE_DIR}/include"
)
target_link_libraries(
  roc_test
  GTest::gtest_main
  ${THIRD_PARTY_LIB}
)

include(GoogleTest)
gtest_discover_tests("${TEST_EXE_NAME}")

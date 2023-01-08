enable_testing()

set(TEST_EXE_NAME roc_test)

file(GLOB_RECURSE TSTFILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/test/*.cpp")

file(GLOB_RECURSE TSTSRCS CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")
get_filename_component(main_file_cpp ../src/main.cpp ABSOLUTE)
list(REMOVE_ITEM TSTSRCS "${main_file_cpp}")

add_executable(
  "${TEST_EXE_NAME}"
  ${TSTFILES}
  ${TSTSRCS}
)

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

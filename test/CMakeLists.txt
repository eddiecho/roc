enable_testing()

set(TEST_EXE_NAME test_main)

file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/test/*.cpp")

file(GLOB_RECURSE TEST_SRCS CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")
get_filename_component(main_file_cpp ../src/main.cpp ABSOLUTE)
list(REMOVE_ITEM TEST_SRCS "${main_file_cpp}")
list(FILTER TEST_SRCS EXCLUDE REGEX ".*(win32|linux|darwin)/.*")

if(WIN32)
  file(GLOB_RECURSE WIN32_SRCS CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/win32/*.cpp")
  list(APPEND TEST_SRCS ${WIN32_SRCS})
elseif(UNIX AND NOT APPLE)
  file(GLOB_RECURSE LINUX_SRCS CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/linux/*.cpp")
  list(APPEND TEST_SRCS ${LINUX_SRCS})
elseif(APPLE)
  file(GLOB_RECURSE DARWIN_SRCS CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/darwin/*.cpp")
  list(APPEND TEST_SRCS ${DARWIN_SRCS})
endif()

add_executable(
  "${TEST_EXE_NAME}"
  ${TEST_FILES}
  ${TEST_SRCS}
)

set_compiler_flags("${TEST_EXE_NAME}")

target_include_directories(
  "${TEST_EXE_NAME}"
  PUBLIC
  "${PROJECT_SOURCE_DIR}/include"
)
target_link_libraries(
  ${TEST_EXE_NAME}
  GTest::gtest_main
  ${THIRD_PARTY_LIB}
)

include(GoogleTest)
gtest_discover_tests("${TEST_EXE_NAME}")

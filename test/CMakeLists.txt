enable_testing()

add_executable(
  roc_test
  roc_test.cpp
)
target_link_libraries(
  roc_test
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(roc_test)

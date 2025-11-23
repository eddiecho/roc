#include <cstdio>
#include <cstring>

#include <gtest/gtest.h>

#include "roc/common.h"

#ifdef _WIN32
#define TEST_DIR "../../../../test"
#else
#define TEST_DIR "../../test"
#endif

auto Fibonacci(u64 n) -> u64 {
  if (n <= 1) return 1;

  u64 prev = 1;
  u64 curr = 1;
  for (u64 i = 1; i < n; i++) {
    prev += curr;

    auto tmp = prev;
    prev = curr;
    curr = tmp;
  }

  return curr;
}

class CompilerTest : public ::testing::Test {
  auto SetUp() -> void override {
  }

  auto TearDown() -> void override {
  }
};

TEST_F(CompilerTest, BasicCompiler) {
  EXPECT_EQ(Fibonacci(4), 5);
}

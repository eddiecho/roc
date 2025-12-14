#include <gtest/gtest.h>

#include "roc/common.h"
#include "roc/ds/arena.h"
#include "roc/ds/array.h"

class ArrayTest : public ::testing::Test {
  auto SetUp() -> void override { this->arena = Alloc(); }

  auto TearDown() -> void override { this->arena->Release(); }

public:
  Arena *arena = nullptr;
};

TEST_F(ArrayTest, Basic) {
  Array<u64> arr = {};
  arr.Init(this->arena);
  for (u64 i = 0; i < 20; i++) {
    arr.Push(i);
  }
}

#include <gtest/gtest.h>

#include "roc/common.h"
#include "roc/ds/arena.h"

class ArenaTest : public ::testing::Test {
  auto SetUp() -> void override {
    this->arena = Alloc();
  }

  auto TearDown() -> void override {
    this->arena->Release();
  }

 public:
  Arena *arena = nullptr;
};

TEST_F(ArenaTest, BasicAlloc) {
  EXPECT_NE(this->arena, nullptr);
  EXPECT_GT(this->arena->reserve, 0);
}

TEST_F(ArenaTest, BasicOps) {
  u64 arr[] = {1, 2, 3, 4};
  auto *ptr = (u64 *)this->arena->Push(sizeof(arr));
  ptr = arr;
  for (int i = 0; i < (sizeof(arr) / sizeof(arr[0])); i++) {
    EXPECT_EQ(arr[i], ptr[i]);
  }

  EXPECT_NE(ptr, nullptr);
  EXPECT_EQ(this->arena->pos, ARENA_HEADER_SIZE + sizeof(arr));

  this->arena->Pop(sizeof(u64));
  EXPECT_NE(this->arena, nullptr);
  EXPECT_EQ(this->arena->pos, ARENA_HEADER_SIZE + sizeof(arr) - sizeof(u64));

  this->arena->Clear();
  EXPECT_NE(this->arena, nullptr);
  EXPECT_NE(this->arena->head, nullptr);
  EXPECT_EQ(this->arena->prev, nullptr);
}

TEST_F(ArenaTest, Overflow) {
  EXPECT_NE(this->arena, nullptr);

  auto *first = this->arena->head;

  auto *ptr = this->arena->Push(MB(1));
  ptr = this->arena->Push(MB(1));
  ptr = this->arena->Push(MB(1));
  ptr = this->arena->Push(MB(1));

  EXPECT_NE(first, this->arena->head);
  EXPECT_NE(this->arena->prev, nullptr);

  this->arena->Clear();
}

#pragma once

#include "roc/common.h"

struct Arena {
  Arena *curr = nullptr;
  Arena *prev = nullptr;
  u64 reserve_size = 0;
  u64 commit_size = 0;
  u64 commit_pos = 0;
  u64 pos = 0;

  const char *alloc_file_location;
  int alloc_file_line;

  auto Release() -> void;
  auto Push(u64 size) -> void*;
  auto Pop(u64 size) -> void;
  auto Clear() -> void;

};

auto Alloc__(const char *file, int line) -> Arena *;
#define Alloc(...) Alloc__(__FILE__, __LINE__)

#define DEFAULT_RESERVE MB(4)
#define DEFAULT_COMMIT KB(64)

// the memory after the fields are where the data is stored
// this is just the smallest power of 2 larger than the size
#define ARENA_HEADER_SIZE 64
static_assert(ARENA_HEADER_SIZE >= sizeof(Arena));
static_assert(DEFAULT_COMMIT >= sizeof(Arena));


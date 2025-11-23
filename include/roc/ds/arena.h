#pragma once

#include "roc/common.h"

struct Arena {
  Arena *curr = nullptr;
  Arena *prev = nullptr;
  u64 reserve_size = 0;
  u64 commit_size = 0;
  u64 pos = 0;

  char *alloc_file_location;
  int alloc_file_line;

  auto Release() -> void;
  auto Push(u64 size) -> void;
  auto Pop(u64 size) -> void;
  auto Clear() -> void;
};

auto Alloc__(char *file, int line) -> Arena *;
#define Alloc(...) Alloc__(__FILE__, __LINE__)

#define DEFAULT_RESERVE MB(4)
#define DEFAULT_COMMIT KB(64)

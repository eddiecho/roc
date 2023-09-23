#pragma once

#include <stddef.h>

#include "common.h"

#define GROW_CAPACITY(cap) ((cap) < 8 ? 8 : (cap)*2)
#define GROW_ARRAY(type, ptr, old_count, new_count) \
  reinterpret_cast<type*>(                          \
      Reallocate(ptr, sizeof(type) * (old_count), sizeof(type) * (new_count)))
#define FREE_ARRAY(type, ptr, count) Reallocate(ptr, sizeof(type) * count, 0)
#define ALLOCATE(type, count) \
  (type*)Reallocate(nullptr, 0, sizeof(type) * (count))

fnc Reallocate(void* ptr, size_t old_size, size_t new_size)->void*;
fnc Collect()->void;
fnc static MarkRoots()->void;

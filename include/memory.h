#pragma once

#include <stddef.h>

#define ALLOCATE(type, count) \
  (type *)Reallocate(nullptr, 0, sizeof(type) * (count))

auto Reallocate(void* ptr, size_t oldSize, size_t newSize) -> void*;

#pragma once

#include <stddef.h>

#include "common.h"

#define ALLOCATE(type, count) \
  (type*)Reallocate(nullptr, 0, sizeof(type) * (count))

fnc Reallocate(void* ptr, size_t oldSize, size_t newSize) -> void*;

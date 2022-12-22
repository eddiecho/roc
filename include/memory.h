#pragma once

#include <stddef.h>

auto Reallocate(void* ptr, size_t oldSize, size_t newSize) -> void*;

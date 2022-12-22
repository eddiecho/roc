#pragma once

#include <cstddef>

auto Reallocate(void* ptr, size_t oldSize, size_t newSize) -> void*;

#include "memory.h"

#include <stdlib.h>

#include "common.h"

// @STDLIB
func Reallocate(void* ptr, size_t oldSize, size_t newSize) -> void* {
  if (newSize == 0) {
    free(ptr);
    return nullptr;
  }

  void* result = realloc(ptr, newSize);
  if (result == nullptr) exit(1);
  return result;
}

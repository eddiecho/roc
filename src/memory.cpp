#include "memory.h"

#include <stdlib.h>

#include "common.h"

// @STDLIB
fnc Reallocate(void* ptr, size_t old_size, size_t new_size) -> void* {
  if (new_size == 0) {
    free(ptr);
    return nullptr;
  }

#ifdef DEBUG_GC_LOG
  printf("%p allocate %zu\n", ptr, new_size);
#endif

  void* result = realloc(ptr, new_size);
  if (result == nullptr) exit(1);
  return result;
}

fnc Collect() -> void {
#ifdef DEBUG_GC_LOG
  printf("-----GC begin\n");
#endif

  MarkRoots();

#ifdef DEBUG_GC_LOG
  printf("-----GC end\n");
#endif
}

fnc static MarkRoots() -> void {}

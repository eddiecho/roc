#include <cstdio>

#include "roc/common.h"
#include "roc/ds/arena.h"
#include "roc/ds/string.h"
#include "roc/os/os.h"

auto main(int argc, char **argv) -> int {
  printf("argc: %d\n", argc);
  for (int i = 0; i < argc; i++) {
    printf("arg[%d]: %s\n", i, argv[i]);
  }

  auto sys_info = Os::GetSysInfo();
  printf("cpu count: %d\n", sys_info.cpu_count);
  printf("page size: %lu\n", sys_info.page_size);

  auto *arena = Alloc();
  defer { arena->Release(); };

  for (int i = 0; i < argc; i++) {
    auto s = NewString(argv[i]);
    s.PrintString();

    auto u = s.Upper(arena);

    printf("copy[%d]: ", i);
    u.PrintString();
    printf("\n");
  }
}

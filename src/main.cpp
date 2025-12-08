#include <cstdio>
#include <print>

#include "roc/common.h"
#include "roc/ds/arena.h"
#include "roc/os/os.h"

auto main(int argc, char **argv) -> int {
  std::print("argc: {}\n", argc);
  for (int i = 0; i < argc; i++) {
    std::print("arg[{}]: {}\n", i, argv[i]);
  }

  auto sys_info = Os::GetSysInfo();
  std::print("cpu count: {}\n", sys_info.cpu_count);
  std::print("page size: {}\n", sys_info.page_size);

  auto *arena = Alloc();
  auto *ptr = reinterpret_cast<char *>(arena->Push(sizeof(argv[0])));
  ptr = argv[0];

  defer { arena->Release(); };

  std::print("copy: {}\n", ptr);
}

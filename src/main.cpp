#include <cstdio>
#include <print>

#include "roc/common.h"
#include "roc/os/os.h"

auto main(int argc, char **argv) -> int {
  std::print("argc: {}\n", argc);
  for (int i = 0; i < argc; i++) {
    std::print("arg[{}]: {}\n", i, argv[i]);
  }

  auto sys_info = Os::GetSystemInfo();
  std::print("cpu count: {}\n", sys_info.cpu_count);
  std::print("page size: {}\n", sys_info.page_size);

  auto *copy = Os::ReserveMemory(sizeof(argv[0]));
  defer { Os::ReleaseMemory(copy, sizeof(argv[0])); };

  std::print("copy: {}\n", copy);
}

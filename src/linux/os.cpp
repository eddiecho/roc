#include "roc/os/os.h"

#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#include "roc/common.h"
#include "roc/os/memory.h"

namespace Os {

auto ReserveMemory(u64 size) -> void * {
  void *result = mmap(0, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (result == MAP_FAILED) {
    return nullptr;
  }

  return result;
}

auto ReleaseMemory(void *ptr, u64 size) -> void { munmap(ptr, size); }

auto CommitMemory(void *ptr, u64 size) -> void {
  mprotect(ptr, size, PROT_READ | PROT_WRITE);
}

auto UncommitMemory(void *ptr, u64 size) -> void {
  madvise(ptr, size, MADV_DONTNEED);
  mprotect(ptr, size, PROT_NONE);
}

auto GetSysInfo() -> SysInfo {
  global SysInfo ret = {};

  if (ret.page_size == 0) {
    auto n = get_nprocs();
    Assert(n > 0);
    ret.cpu_count = static_cast<u32>(n);
    auto p = getpagesize();
    Assert(p > 0);
    ret.page_size = static_cast<u64>(p);
    ret.kind = Os::Kind::Linux;
  }

  return ret;
}

} // namespace Os

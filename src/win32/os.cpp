#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "roc/os/memory.h"
#include "roc/os/os.h"

namespace Os {

auto ReserveMemory(u64 size) -> void * {
  return VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
}

auto ReleaseMemory(void *ptr, u64 size) -> void {
  // Windows is weird like that
  VirtualFree(ptr, 0, MEM_RELEASE);
}

auto CommitMemory(void *ptr, u64 size) -> void {
  // apparently you need to do something with sockets to decommit?
  // see RIODeregisterBuffer in MSDN
  VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
}

auto UncommitMemory(void *ptr, u64 size) -> void {
  VirtualFree(ptr, size, MEM_DECOMMIT);
}

auto GetSysInfo() -> SysInfo {
  global SysInfo ret = {};

  if (ret.page_size == 0) {
    SYSTEM_INFO sysinfo = {};
    GetSystemInfo(&sysinfo);

    ret.cpu_count = sysinfo.dwNumberOfProcessors;
    ret.page_size = sysinfo.dwPageSize;
    ret.kind = Os::Kind::Windows;
  }

  return ret;
}

} // namespace Os

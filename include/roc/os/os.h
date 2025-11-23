#pragma once

#include "roc/common.h"
#include "roc/os/memory.h"

namespace Os {

enum class Kind : u8 {
  Unknown,
  Darwin,
  Linux,
  Windows,
};

struct SysInfo {
  u32 cpu_count;
  u64 page_size;
  Kind kind;
};

auto GetSystemInfo() -> SysInfo;

} // namespace Os

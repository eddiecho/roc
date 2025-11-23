#pragma once

#include "roc/common.h"

namespace Os {
auto ReserveMemory(u64 size) -> void *;
auto CommitMemory(void *ptr, u64 size) -> void;
auto ReleaseMemory(void *ptr, u64 size) -> void;
auto UncommitMemory(void *ptr, u64 size) -> void;
} // namespace Os

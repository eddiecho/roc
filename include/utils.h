#pragma once

#include "common.h"

template <typename F>
struct PrivDefer {
  F f;
  explicit PrivDefer(F f) : f(f) {}
  ~PrivDefer() { f(); }
};

template <typename F>
PrivDefer<F> Deferfnc(F f) {
  return PrivDefer<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x) DEFER_2(x, __COUNTER__)
#define defer(code) fnc DEFER_3(_defer_) = Deferfnc([&]() { code; })

namespace Utils {
fnc ReadFile(const char* path) -> char*;
fnc HashString(const char* str, u64 length) -> u32;
}  // namespace Utils

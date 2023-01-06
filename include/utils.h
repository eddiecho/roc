#pragma once

#include "common.h"

template <typename F>
struct PrivDefer {
  F f;
  explicit PrivDefer(F f) : f(f) {}
  ~PrivDefer() { f(); }
};

template <typename F>
PrivDefer<F> DeferFunc(F f) {
  return PrivDefer<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x) DEFER_2(x, __COUNTER__)
#define defer(code) func DEFER_3(_defer_) = DeferFunc([&]() { code; })

namespace Utils {
func ReadFile(const char* path) -> char*;
}  // namespace Utils

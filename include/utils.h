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

#if DEBUG
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

namespace Utils {
fnc ReadFile(const char* path) -> char*;

fnc constexpr inline HashString(const char* str, u64 length) -> u32 {
  u32 hash = 2166136261u;

  for (u32 i = 0; i < length; i++) {
    hash ^= (u8)str[i];
    hash *= 16777619;
  }

  return hash;
}

constexpr u32 EMPTY_STRING_HASH = HashString("", 0);
}  // namespace Utils

template <typename T, typename E>
struct Result {
  enum class ResultType {
    Some,
    Error,
  };

  ResultType type;
  union {
    T obj;
    E err;
  } as;

  Result<T, E>() {
    this->type = ResultType::Some;
  };

  Result<T, E>(E err) {
    this->type = ResultType::Error;
    this->as.err = err;
  };

  Result<T, E>(T t) {
    this->type = ResultType::Some;
    this->as.obj = t;
  };

  fnc IsError() -> bool {
    return this->type == ResultType::Error;
  }

  fnc Get() -> T {
    Assert(this->type == ResultType::Some);
    return this->as.obj;
  }
};

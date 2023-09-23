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
#define Assert(Expression) \
  if (!(Expression)) {     \
    *(int*)0 = 0;          \
  }
#else
#define Assert(Expression)
#endif

#define IntToBytes(num) reinterpret_cast<u8*>(num)

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
    Ok,
    Error,
  };

  ResultType type;
  union Data {
    T obj;
    E err;

    Data() { memset(this, 0, sizeof(Data)); }
    ~Data() {}
  } as;

  Result<T, E>() { this->type = ResultType::Ok; };

  Result<T, E>(E err) {
    this->type = ResultType::Error;
    this->as.err = err;
  };

  Result<T, E>(T t) {
    this->type = ResultType::Ok;
    this->as.obj = t;
  };

  fnc IsError() const->bool { return this->type == ResultType::Error; }

  fnc Get() const->T {
    Assert(this->type == ResultType::Ok);
    return this->as.obj;
  }

  fnc Err() const->E {
    Assert(this->type == ResultType::Error);
    return this->as.err;
  }
};

enum class OptionType {
  Some,
  None,
};

template <typename T>
struct Option {
  OptionType type;
  T data = {};

  // this is just to faciliate returning None without using empty constructor
  // you definitely shouldnt use it to return Some
  // but you know, cpp is stupid
  Option<T>(OptionType type) {
    Assert(type != OptionType::Some);
    this->type = type;
  };

  Option<T>() { this->type = OptionType::None; }

  Option<T>(T t) {
    this->type = OptionType::Some;
    this->data = t;
  }

  fnc IsNone() const->bool { return this->type == OptionType::None; }

  fnc Get() const->T {
    Assert(this->type == OptionType::Some);
    return this->data;
  }

  operator bool() { return !this->IsNone(); }
};

#define c_macro_var(name) concat(name, __LINE__)
#define c_defer(start, end) for (    \
  int c_macro_var(_i_) = (start, 0); \
  !c_macro_var(_i_);                 \
  (c_macro_var(_i_) += 1, end)

/*
c_defer(begin(), end()) {

}
*/

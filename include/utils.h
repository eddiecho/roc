#pragma once

#include <string>
#include <type_traits>
#include <utility>

#include <stdint.h>

#define i64 int64_t
#define u64 uint64_t

#define func auto

template <typename _Fx>
struct __defer_t {
  _Fx __fx;

  __defer_t(_Fx&& __arg_fx)
  noexcept(::std::is_nothrow_move_constructible_v<_Fx>)
  : __fx(::std::move(__arg_fx)) {}

  ~__defer_t()
  noexcept(::std::is_nothrow_invocable_v<_Fx>) {
    __fx();
  }
};

template <typename _Fx>
__defer_t(_Fx __fx) -> __defer_t<::std::decay_t<_Fx>>;

#define __DEFER_TOK_CONCAT(X, Y) X ## Y
#define __DEFER_TOK_PASTE(X, Y) __DEFER_TOK_CONCAT(X, Y)
#define defer __defer_t \
  __DEFER_TOK_PASTE(__scoped_defer_obj, __COUNTER__) = \
  [&]()

/**
* should print "cat says meow :3"

int main() {
  defer {
    defer {
      printf(" :3");
    };
    printf(" meow");
  };

  printf("cat says");
  return 0;
}
*/

namespace utils {

func static inline constexpr len(const char* s) -> u64 {
  return std::char_traits<char>::length(s);
}

}

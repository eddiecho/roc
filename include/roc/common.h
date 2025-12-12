#pragma once

#include <cctype>
#include <cstdint>
#include <type_traits>
#include <utility>

using b32 = int32_t;
using u8 = unsigned char;
using i32 = int32_t;
using i64 = int64_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;

#define AlignPow2(x, b) (((x) + (b) - 1) & (~((b) - 1)))
#define KB(n) (((u64)(n)) << 10)
#define MB(n) (((u64)(n)) << 20)
#define GB(n) (((u64)(n)) << 30)
#define TB(n) (((u64)(n)) << 40)

#define global static

#define Stringify_(S) #S
#define Stringify(S) Stringify_(S)

#define Concat_(A, B) A##B
#define Concat(A, B) Concat_(A, B)

#if defined(__GNUC__) && !defined(__clang__)
#define COMPILER_GCC
#endif

#ifdef __clang__
#define COMPILER_CLANG
#endif

#ifdef _MSC_VER
#define COMPILER_MSVC
#endif

#ifdef COMPILER_MSVC
#define Trap() __debugbreak()
#elifdef COMPILER_CLANG
#define Trap() __builtin_trap()
#elifdef COMPILER_GCC
#define Trap() __builtin_trap()
#endif

#define Assert(expr)                                                           \
  do {                                                                         \
    if (!(expr)) {                                                             \
      Trap();                                                                  \
    }                                                                          \
  } while (0)

template <typename _Fx> struct __defer_t {
  _Fx __fx;

  __defer_t(_Fx &&__arg_fx) noexcept(
      ::std::is_nothrow_move_constructible_v<_Fx>)
      : __fx(::std::move(__arg_fx)) {}

  ~__defer_t() noexcept(::std::is_nothrow_invocable_v<_Fx>) { __fx(); }
};

template <typename _Fx> __defer_t(_Fx __fx) -> __defer_t<::std::decay_t<_Fx>>;

#define defer __defer_t Concat(__scoped_defer_obj, __COUNTER__) = [&]()

#define Min(a, b) ((a < b) ? a : b)
#define Max(a, b) ((a > b) ? a : b)

#define PtrFromInt(i)               (void*)(i)
#define IntFromPtr(ptr)             ((u64)(ptr))
#define Member(T,m)                 (((T*)0)->m)
#define OffsetOf(T,m)               IntFromPtr(&Member(T,m))
#define MemberFromOffset(T,ptr,off) (T)((((u8 *)ptr)+(off)))
#define CastFromMember(T,m,ptr)     (T*)(((u8*)ptr) - OffsetOf(T,m))

#pragma once

#include <format>

#include "roc/common.h"
#include "roc/ds/arena.h"

struct String {
  u64 len;
  const char *raw;

  auto Lower(Arena *arena) -> String;
  auto Upper(Arena *arena) -> String;

  auto Substring(Arena *arena,u64 lo, u64 hi) -> String;
  auto Prefix(Arena *arena, u64 end) -> String;
  auto Suffix(Arena *arena, u64 start) -> String;
};

template <>
struct std::formatter<String> {
  constexpr auto parse(std::format_parse_context& ctx) {
    return ctx.begin();
  }

  auto format(const String& obj, std::format_context& ctx) const {
    return std::format_to_n(ctx.out(), obj.len, "{}", obj.raw);
  }
};

auto NewString() -> String;
auto NewString(char *raw) -> String;
auto NewString(char *raw, u64 size) -> String;

auto IsUpper(char c) -> b32;
auto IsLower(char c) -> b32;
auto IsWhitespace(char c) -> b32;
auto IsAlpha(char c) -> b32;
auto IsNum(char c) -> b32;
auto IsAlnum(char c) -> b32;
auto IsPunct(char c) -> b32;
auto IsIdentifier(char c) -> b32;

auto ToUpper(char c) -> char;
auto ToLower(char c) -> char;

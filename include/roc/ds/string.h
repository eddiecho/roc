#pragma once

#include "roc/common.h"
#include "roc/ds/arena.h"

struct String {
  u64 len;
  const char *raw;

  auto Print() -> const char*;
  auto Lower(Arena *arena) -> String;
  auto Upper(Arena *arena) -> String;

  auto Substring(Arena *arena,u64 lo, u64 hi) -> String;
  auto Prefix(Arena *arena, u64 end) -> String;
  auto Suffix(Arena *arena, u64 start) -> String;
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

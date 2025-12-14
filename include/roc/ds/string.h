#pragma once

#include "roc/common.h"
#include "roc/ds/arena.h"

// Not necessarily null-terminated
// If in doubt, use the PrintString() method
// No SSO because I'm stupid
// But in practice, any Arena based methods will alloc next to the struct itself
struct String {
  u64 len;
  const char *ptr;

  auto PrintString() -> void;
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

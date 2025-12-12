#include "roc/ds/string.h"

#include <cctype>
#include <cstring>

#include "roc/ds/arena.h"

auto inline IsUpper(char c) -> b32 {
  return isupper(c);
}

auto inline IsLower(char c) -> b32 {
  return islower(c);
}

auto inline IsWhitespace(char c) -> b32 {
  return isblank(c);
}

auto inline IsAlpha(char c) -> b32 {
  return isalpha(c);
}

auto inline IsNum(char c) -> b32 {
  return isdigit(c);
}

auto inline IsAlnum(char c) -> b32 {
  return isalnum(c);
}

auto inline IsPunct(char c) -> b32 {
  return ispunct(c);
}

auto inline IsIdentifier(char c) -> b32 {
  return !ispunct(c) && !isblank(c);
}

auto inline ToUpper(char c) -> char {
  return toupper(c);
}

auto inline ToLower(char c) -> char {
  return tolower(c);
}

auto NewString() -> String {
  String ret = {};
  return ret;
}

auto NewString(char *c) -> String {
  String ret = {};
  ret.len = strlen(c); // reminder that strlen doesn't include the null terminator
  ret.raw = c;
  return ret;
}

auto NewString(char *c, u64 size) -> String {
  String ret = {};
  ret.len = size;
  ret.raw = c;
  return ret;
}

auto String::Lower(Arena *arena) -> String {
  auto *ptr = reinterpret_cast<String*>(arena->Push(sizeof(u64) + this->len));
  ptr->len = this->len;
  // memcpy(ptr + OffsetOf(String, raw), this->raw, this->len);

  auto* dst_cur = (char*)ptr + OffsetOf(String, raw);
  auto* src_cur = this->raw;
  for (u64 i = 0; i < this->len; i++) {
    // *cursor = ToLower(*cursor++);
    *dst_cur++ = ToLower(*src_cur++);
  }

  return *ptr;
}

auto String::Upper(Arena *arena) -> String {
  auto *ptr = reinterpret_cast<String*>(arena->Push(sizeof(u64) + this->len));
  ptr->len = this->len;
  // memcpy(ptr + OffsetOf(String, raw), this->raw, this->len);

  auto* dst_cur = (char*)ptr + OffsetOf(String, raw);
  auto* src_cur = this->raw;
  for (u64 i = 0; i < this->len; i++) {
    *dst_cur++ = ToUpper(*src_cur++);
  }

  return *ptr;
}

auto String::Substring(Arena *arena, u64 lo, u64 hi) -> String {
  return *this;
}

auto String::Prefix(Arena *arena, u64 end) -> String {
  return *this;
}

auto String::Suffix(Arena *arena, u64 start) -> String {
  return *this;
}

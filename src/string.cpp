#include "roc/ds/string.h"

#include <cctype>
#include <cstdio>
#include <cstring>

#include "roc/ds/arena.h"

auto inline IsUpper(char c) -> b32 { return isupper(c); }

auto inline IsLower(char c) -> b32 { return islower(c); }

auto inline IsWhitespace(char c) -> b32 { return isblank(c); }

auto inline IsAlpha(char c) -> b32 { return isalpha(c); }

auto inline IsNum(char c) -> b32 { return isdigit(c); }

auto inline IsAlnum(char c) -> b32 { return isalnum(c); }

auto inline IsPunct(char c) -> b32 { return ispunct(c); }

auto inline IsIdentifier(char c) -> b32 { return !ispunct(c) && !isblank(c); }

auto inline ToUpper(char c) -> char { return (char)toupper(c); }

auto inline ToLower(char c) -> char { return (char)tolower(c); }

auto NewString() -> String {
  String ret = {};
  return ret;
}

auto NewString(char *c) -> String {
  String ret = {};
  ret.len =
      strlen(c); // reminder that strlen doesn't include the null terminator
  ret.ptr = c;
  return ret;
}

auto NewString(char *c, u64 size) -> String {
  String ret = {};
  ret.len = size;
  ret.ptr = c;
  return ret;
}

auto String::PrintString() -> void {
  printf("(%.*s)\n", (int)this->len, this->ptr);
}

auto String::Copy(Arena *arena) -> String {
  String ret = {};
  ret.len = this->len;

  auto *dst_cur = reinterpret_cast<char *>(arena->Push(this->len));
  ret.ptr = dst_cur;

  auto *src_cur = this->ptr;
  for (u64 i = 0; i < this->len; i++) {
    *dst_cur++ = *src_cur++;
  }

  return ret;
}

auto String::Lower(Arena *arena) -> String {
  String ret = {};
  ret.len = this->len;

  auto *dst_cur = reinterpret_cast<char *>(arena->Push(this->len));
  ret.ptr = dst_cur;

  auto *src_cur = this->ptr;
  for (u64 i = 0; i < this->len; i++) {
    *dst_cur++ = ToLower(*src_cur++);
  }

  return ret;
}

auto String::Upper(Arena *arena) -> String {
  String ret = {};
  ret.len = this->len;

  auto *dst_cur = reinterpret_cast<char *>(arena->Push(this->len));
  ret.ptr = dst_cur;

  auto *src_cur = this->ptr;
  for (u64 i = 0; i < this->len; i++) {
    *dst_cur++ = ToUpper(*src_cur++);
  }

  return ret;
}

auto String::Substring(u64 lo, u64 hi) -> String {
  String ret = {};
  Assert(hi >= lo);
  if (hi == lo) {
    return ret;
  }

  ret.len = hi - lo;
  ret.ptr = this->ptr + lo;

  return ret;
}

auto String::Prefix(u64 end) -> String { return this->Substring(0, end); }

auto String::Suffix(u64 start) -> String {
  return this->Substring(start, this->len);
}

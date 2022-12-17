#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <cctype>

template <typename F>
struct privDefer {
  F f;
  explicit privDefer(F f) : f(f) {}
  ~privDefer() { f(); }
};

template <typename F>
privDefer<F> defer_func(F f) {
  return privDefer<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) = defer_func([&](){code;})

namespace Utils {

auto static ReadFile(const char* path) -> char* {
  // @STDLIB
  FILE* file;
#ifdef _WIN32
  errno_t err = fopen_s(&file, path, "rb");
  if (err != 0) {
    fprintf(stderr, "Could not open file \"%s\".\n", path);
    exit(75);
  }
#else
  file = fopen(path, "rb");

  if (file == NULL) {
    fprintf(stderr, "Could not open file \"%s\".\n", path);
    exit(75);
  }
#endif

  defer(fclose(file));

  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  char* buffer = reinterpret_cast<char*>(malloc(fileSize + 1));
  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
  buffer[bytesRead] = '\0';

  return buffer;
}

auto static IsDigit(char c) -> bool {
  return c >= '0' && c <= '9';
}

auto static IsIdentifier(char c) -> bool {
  return !ispunct(c);
}

}  // namespace Utils

#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

// @STDLIB
auto Utils::ReadFile(const char* path) -> char* {
  FILE* file;

#ifdef _WIN32
  errno_t err = fopen_s(&file, path, "rb");
  if (err != 0) {
    fprintf(stderr, "Could not open file \"%s\".\n", path);
    exit(75);
  }
#else
  file = fopen(path, "rb");
  if (file == nullptr) {
    fprintf(stderr, "Could not open file \"%s\".\n", path);
    exit(75);
  }
#endif

  defer(fclose(file));

  fseek(file, 0L, SEEK_END);
  size_t file_size = ftell(file);
  rewind(file);

  char* buffer = reinterpret_cast<char*>(malloc(file_size + 1));
  size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
  buffer[bytes_read] = '\0';

  return buffer;
}

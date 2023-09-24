#pragma once

#include <string>

#include "absl/container/flat_hash_map.h"
#include "arena.h"
#include "common.h"
#include "dynamic_array.h"
#include "object.h"

#define INIT_STR_POOL_SIZE 1024

class StringPool {
 public:
  auto Init(Arena<Object>* object_pool) -> void;
  auto Deinit() -> void;
  auto Alloc(u64 length, const char* start) -> u64;
  auto Nth(u64 idx) -> Object*;

 private:
  DynamicArray<char>* char_data = new DynamicArray<char>();
  Arena<Object>* object_pool = nullptr;
  absl::flat_hash_map<std::string_view, u64> intern_table;
};



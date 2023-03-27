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
  fnc Init(Arena<Object>* object_pool) -> void;
  fnc Deinit() -> void;
  fnc Alloc(u64 length, const char* start) -> u64;
  fnc Nth(u64 idx) -> Object*;

 private:
  DynamicArray<char>* char_data = new DynamicArray<char>();
  Arena<Object>* object_pool = nullptr;
  absl::flat_hash_map<std::string_view, u64> intern_table;
};



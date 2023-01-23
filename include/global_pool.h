#pragma once

#include "absl/container/flat_hash_map.h"

#include "arena.h"
#include "common.h"
#include "dynamic_array.h"
#include "object.h"

class GlobalPool {
 public:
  fnc Init(Arena<Object>* object_pool) -> void;
  fnc Deinit() -> void;
  fnc Alloc(u32 length, const char* start) -> u32;
  fnc Nth(u32 idx) -> Object*;
  fnc Find(u32 length, const char* start) -> u32;

  static constexpr u32 INVALID_INDEX = 0xFFFFFFFF;

 private:
  Arena<Object>* object_pool = nullptr;
  absl::flat_hash_map<std::string, u32> index;
};

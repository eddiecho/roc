#pragma once

#include "absl/container/flat_hash_map.h"

#include "arena.h"
#include "common.h"
#include "dynamic_array.h"
#include "object.h"

using KeyType = std::string;

class GlobalPool {
 public:
  fnc Init(Arena<Object>* object_pool) -> void;
  fnc Deinit() -> void;
  fnc Alloc(u64 length, const char* start, ObjectType obj_type) -> u64;
  fnc Nth(u64 idx) -> Object*;
  fnc Find(u64 length, const char* start) -> u64;

  static constexpr u64 INVALID_INDEX = 0xFFFFFFFF;

 private:
  Arena<Object>* object_pool = nullptr;
  absl::flat_hash_map<KeyType, u64> index;
};

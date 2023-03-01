#pragma once

#include <utility>

#include "absl/container/flat_hash_map.h"

#include "arena.h"
#include "common.h"
#include "dynamic_array.h"
#include "object.h"

#define GLOBAL_POOL_INVALID_INDEX 0xFFFFFFFF

using KeyType = std::string;
struct PossibleObjects {
  u64 indices[OBJECT_TYPE_COUNT] = {GLOBAL_POOL_INVALID_INDEX, GLOBAL_POOL_INVALID_INDEX};
};

class GlobalPool {
 public:
  fnc Init(Arena<Object>* object_pool) -> void;
  fnc Deinit() -> void;
  fnc Alloc(u64 length, const char* start, ObjectType obj_type) -> u64;
  fnc Nth(u64 idx) -> Object*;
  fnc Find(u64 length, const char* start) -> u64;

  static constexpr u64 INVALID_INDEX = GLOBAL_POOL_INVALID_INDEX;

 private:
  Arena<Object>* object_pool = nullptr;
  absl::flat_hash_map<KeyType, u64> index;
};

#undef GLOBAL_POOL_INVALID_INDEX

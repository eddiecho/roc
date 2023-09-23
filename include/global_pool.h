#pragma once

#include <memory>

#include "absl/container/flat_hash_map.h"
#include "arena.h"
#include "common.h"
#include "dynamic_array.h"
#include "object.h"

// @TODO(eddie) - make KeyType integral, because that means
// we don't need to recalculate the hash everytime
using KeyType = std::string_view;

class GlobalPool {
 public:
  fnc Init(Arena<Object>* object_pool) -> void;
  fnc Deinit() -> void;
  fnc Alloc(u64 length, const char* start) -> u64;
  fnc Nth(u64 idx) -> Object*;
  fnc Find(u64 length, const char* start) -> Option<u64>;

 private:
  Arena<Object>* object_pool = nullptr;
  absl::flat_hash_map<KeyType, u64> index;
};

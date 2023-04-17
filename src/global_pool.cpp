#include "global_pool.h"

#include <memory>
#include <string>

#include "common.h"
#include "object.h"

fnc GlobalPool::Init(Arena<Object>* object_pool) -> void {
  this->object_pool = object_pool;
}

fnc GlobalPool::Deinit() -> void {
  this->object_pool = nullptr;
  this->index.clear();
}

fnc GlobalPool::Alloc(u64 length, const char* start) -> u64 {
  auto idx = this->Find(length, start);

  if (!idx.IsNone()) {
    return idx.Get();
  }

  KeyType key = {start, length};

  auto obj_idx = this->object_pool->Alloc();
  this->index.emplace(key, obj_idx);

  return obj_idx;
}

fnc GlobalPool::Find(u64 length, const char* start) -> Option<u64> {
  KeyType key = {start, length};
  auto it = this->index.find(key);
  if (it != this->index.end()) {
    return it->second;
  }

  return OptionType::None;
}

fnc GlobalPool::Nth(u64 index) -> Object* {
  return this->object_pool->Nth(index);
}

#include "global_pool.h"

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

fnc GlobalPool::Alloc(u64 length, const char* start, ObjectType obj_type) -> u64 {
  u64 idx = this->Find(length, start, obj_type);
  if (idx != GlobalPool::INVALID_INDEX) {
    return idx;
  }

  std::string str {start, length};
  KeyType key = { str, obj_type };

  auto obj_idx = this->object_pool->Alloc();
  this->index.emplace(key, obj_idx);

  return obj_idx;
}

fnc GlobalPool::Find(u64 length, const char* start, ObjectType obj_type) -> u64 {
  std::string str {start, length};
  KeyType key = { str, obj_type };

  auto it = this->index.find(key);
  if (it == this->index.end()) {
    return GlobalPool::INVALID_INDEX;
  }

  return it->second;
}

fnc GlobalPool::Nth(u64 index) -> Object* {
  return this->object_pool->Nth(index);
}

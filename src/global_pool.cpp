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

fnc GlobalPool::Alloc(u32 length, const char* start) -> u32 {
  std::string str {start, length};
  u32 idx = this->Find(length, start);
  if (idx != GlobalPool::INVALID_INDEX) {
    return idx;
  }

  auto obj_idx = this->object_pool->Alloc();
  this->index.emplace(str, obj_idx);

  return obj_idx;
}

fnc GlobalPool::Find(u32 length, const char* start) -> u32 {
  std::string str {start, length};
  auto it = this->index.find(str);
  if (it == this->index.end()) {
    return GlobalPool::INVALID_INDEX;
  }

  return it->second;
}

fnc GlobalPool::Nth(u32 index) -> Object* {
  return this->object_pool->Nth(index);
}

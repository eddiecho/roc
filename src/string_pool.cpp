#include "string_pool.h"

#include <string>

#include "common.h"
#include "object.h"

fnc StringPool::Init(Arena<Object>* object_pool) -> void {
  this->object_pool = object_pool;
  this->char_data->Init(INIT_STR_POOL_SIZE);
}

fnc StringPool::Deinit() -> void {
  this->object_pool = nullptr;
  this->char_data->Deinit();
  this->intern_table.clear();
}

fnc StringPool::Alloc(u64 length, const char* start) -> u64 {
  std::string_view str {start, length};
  auto it = this->intern_table.find(str);
  if (it != this->intern_table.end()) {
    return it->second;
  }

  auto obj_idx = this->object_pool->Alloc();
  auto *obj = static_cast<Object::String*>(this->object_pool->Nth(obj_idx));

  u64 data_ptr = this->char_data->Append(const_cast<char*>(start), length);
  this->char_data->Append(0);
  obj->Init(length, this->char_data->data + data_ptr);

  this->intern_table.emplace(str, obj_idx);

  return obj_idx;
}

fnc StringPool::Nth(u64 index) -> Object* {
  return this->object_pool->Nth(index);
}


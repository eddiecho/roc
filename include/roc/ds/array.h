#pragma once

#include "roc/common.h"
#include "roc/ds/arena.h"

template <class T> class Array {
  u64 size;
  u64 capacity_in_bytes;
  T *ptr;

public:
  auto Init(Arena *arena) -> void;
  auto Deinit() -> void;
  auto Push(T t) -> void;
  auto Nth(u64 idx) -> T *;
};

template <class T> auto Array<T>::Init(Arena *arena) -> void {
  this->capacity_in_bytes = DEFAULT_COMMIT;
  this->size = 0;

  auto dst = arena->Push(this->capacity_in_bytes);
  this->ptr = reinterpret_cast<T *>(dst);
}

template <class T> auto Array<T>::Deinit() -> void { this->size = 0; }

template <class T> auto Array<T>::Push(T obj) -> void {
  auto used = this->size * (sizeof(T));

  // TODO - handle relocation
  if (this->capacity_in_bytes - used < sizeof(T)) {
  }

  T *dst = this->ptr + this->size++;
  *dst = obj;
}

template <class T> auto Array<T>::Nth(u64 idx) -> T * {
  Assert(idx < size);
  return this->ptr[idx];
}

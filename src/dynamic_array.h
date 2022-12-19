#pragma once

#include "common.h"
#include "memory.h"

#define GROW_CAPACITY(cap) \
  ((cap) < 8 ? 8 : (cap) * 2)
#define GROW_ARRAY(type, ptr, oldC, newC) \
  reinterpret_cast<type*>(Reallocate(ptr, sizeof(type) * (oldC), sizeof(type) * (newC)))
#define FREE_ARRAY(type, ptr, count) \
  Reallocate(ptr, sizeof(type) * count, 0)

template <typename T>
class DynamicArray {
 public:
  u32 count;
  T* data;

  auto Init() -> void;
  auto Append(T item) -> void;
  auto Deinit() -> void;

  auto operator[](std::size_t idx) -> T& {
    return this->data[idx];
  }
  const T& operator[](std::size_t idx) const {
    return this->data[idx];
  }

 private:
  u32 capacity_;

};

template <typename T>
auto DynamicArray<T>::Init() -> void {
  this->count = 0;
  this->capacity_ = 0;
  this->data = NULL;
}

template <typename T>
auto DynamicArray<T>::Append(T item) -> void {
  if (this->capacity_ < this->count + 1) {
    int oldCapacity = this->capacity_;
    this->capacity_ = GROW_CAPACITY(oldCapacity);
    typedef T type;
    this->data = GROW_ARRAY(type, this->data, oldCapacity, this->capacity_);
  }

  this->data[this->count] = item;
  this->count++;
}

template <typename T>
auto DynamicArray<T>::Deinit() -> void {
  typedef T type;
  FREE_ARRAY(type, this->data, this->capacity_);
  this->Init();
}

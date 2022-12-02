#pragma once

#include "common.h"
#include "memory.h"

#define GROW_CAPACITY(cap) \
  ((cap) < 8 ? 8 : (cap) * 2)
#define GROW_ARRAY(type, ptr, oldC, newC) \
  (type*)Reallocate(ptr, sizeof(type) * (oldC), sizeof(type) * (newC))
#define FREE_ARRAY(type, ptr, count) \
  Reallocate(ptr, sizeof(type) * count, 0)

template <typename T>
class DynamicArray {

public:
  auto Init() -> void;
  auto Append(T item) -> void;
  auto Deinit() -> void;

  auto operator[](std::size_t idx) -> T& {
    return data_[idx];
  }
  const T& operator[](std::size_t idx) const {
    return data_[idx];
  }

  u32 count_;
  u32 capacity_;
  T* data_;

};

template <typename T>
auto DynamicArray<T>::Init() -> void {
  this->count_ = 0;
  this->capacity_ = 0;
  this->data_ = NULL;
}

template <typename T>
auto DynamicArray<T>::Append(T item) -> void {
  if (this->capacity_ < this->count_ + 1) {
    int oldCapacity = this->capacity_;
    this->capacity_ = GROW_CAPACITY(oldCapacity);
    typedef T type;
    this->data_ = GROW_ARRAY(type, this->data_, oldCapacity, this->capacity_);
  }

  this->data_[this->count_] = item;
  this->count_++;
}

template <typename T>
auto DynamicArray<T>::Deinit() -> void {
  typedef T type;
  FREE_ARRAY(type, this->data_, this->capacity_);
  this->Init();
}

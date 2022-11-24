#pragma once

#include "common.h"
#include "memory.h"

#define GROW_CAPACITY(cap) \
  ((cap) < 8 ? 8 : (cap) * 2)
#define GROW_ARRAY(type, ptr, oldC, newC) \
  (type*)reallocate(ptr, sizeof(type) * (oldC), sizeof(type) * (newC))
#define FREE_ARRAY(type, ptr, count) \
  reallocate(ptr, sizeof(type) * count, 0)

template <typename T>
struct DynamicArray {
  u32 count;
  u32 capacity;
  T* data;

  auto init() -> void;
  auto append(T item) -> void;
  auto deinit() -> void;

  auto operator[](std::size_t idx) -> T& {
    return data[idx];
  }
  const T& operator[](std::size_t idx) const {
    return data[idx];
  }
};

template <typename T>
auto DynamicArray<T>::init() -> void {
  this->count = 0;
  this->capacity = 0;
  this->data = NULL;
}

template <typename T>
auto DynamicArray<T>::append(T item) -> void {
  if (this->capacity < this->count + 1) {
    int oldCapacity = this->capacity;
    this->capacity = GROW_CAPACITY(oldCapacity);
    typedef T type;
    this->data = GROW_ARRAY(type, this->data, oldCapacity, this->capacity);
  }

  this->data[this->count] = item;
  this->count++;
}

template <typename T>
auto DynamicArray<T>::deinit() -> void {
  typedef T type;
  FREE_ARRAY(type, this->data, this->capacity);
  this->init();
}

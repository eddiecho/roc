#pragma once

#include <cstring>

#include "common.h"
#include "memory.h"

#define GROW_CAPACITY(cap) ((cap) < 8 ? 8 : (cap)*2)
#define GROW_ARRAY(type, ptr, oldC, newC) \
  reinterpret_cast<type*>(                \
      Reallocate(ptr, sizeof(type) * (oldC), sizeof(type) * (newC)))
#define FREE_ARRAY(type, ptr, count) Reallocate(ptr, sizeof(type) * count, 0)

template <typename T>
class DynamicArray {
 public:
  u32 count;
  T* data;

  func Init() -> void;
  func Init(u32 size) -> void;
  func Append(T item) -> u32;
  func Append(T* items, u32 size) -> u32;
  func Deinit() -> void;

  func operator[](size_t idx) -> T& { return this->data[idx]; }
  func operator[](size_t idx) const -> const T& { return this->data[idx]; }

 private:
  u64 capacity_;
};

template <typename T>
func DynamicArray<T>::Init() -> void {
  this->count = 0;
  this->capacity_ = 0;
  this->data = nullptr;
}

template <typename T>
func DynamicArray<T>::Init(u32 size) -> void {
  this->count = 0;
  this->capacity_ = size;
  using type = T;
  this->data = GROW_ARRAY(type, nullptr, 0, size);
}

template <typename T>
func DynamicArray<T>::Append(T item) -> u32 {
  if (this->capacity_ < this->count + 1) {
    int old_capacity = this->capacity_;
    this->capacity_ = GROW_CAPACITY(old_capacity);
    using type = T;
    this->data = GROW_ARRAY(type, this->data, old_capacity, this->capacity_);
  }

  this->data[this->count] = item;
  this->count++;

  return this->count - 1;
}

template <typename T>
func DynamicArray<T>::Append(T* items, u32 size) -> u32 {
  using type = T;

  while (this->capacity_ < this->count + size) {
    int old_capacity = this->capacity_;
    this->capacity_ = GROW_CAPACITY(old_capacity);

    this->data = GROW_ARRAY(type, this->data, old_capacity, this->capacity_);
  }

  std::memcpy(&this->data[this->count], items, sizeof(type) * size);
  this->count += size;

  return this->count - size;
}

template <typename T>
func DynamicArray<T>::Deinit() -> void {
  using type = T;
  FREE_ARRAY(type, this->data, this->capacity_);
  this->Init();
}

#pragma once

#include <cstring>

#include "common.h"
#include "memory.h"

#define DEFAULT_SIZE 1024

template <typename T>
class DynamicArray {
 public:
  DynamicArray<T>() noexcept;

  auto Init() -> void;
  auto Init(u64 size) -> void;
  auto Append(T item) -> u64;
  auto Append(T* items, u64 size) -> u64;
  auto Deinit() -> void;

  auto operator[](size_t idx) -> T& { return this->data[idx]; }
  auto operator[](size_t idx) const -> const T& { return this->data[idx]; }

 public:
  u64 count;
  T* data = nullptr;

 private:
  u64 capacity_;
};

template <typename T>
DynamicArray<T>::DynamicArray() noexcept {
  this->count = 0;
  this->capacity_ = 0;
  this->data = nullptr;
}

template <typename T>
auto DynamicArray<T>::Init() -> void {
  this->count = 0;
  this->capacity_ = 0;
  this->data = nullptr;
}

template <typename T>
auto DynamicArray<T>::Init(u64 size) -> void {
  this->count = 0;
  this->capacity_ = size;
  this->data = GROW_ARRAY(T, nullptr, 0, size);
}

template <typename T>
auto DynamicArray<T>::Append(T item) -> u64 {
  if (this->capacity_ < this->count + 1) {
    u64 old_capacity = this->capacity_;
    this->capacity_ = GROW_CAPACITY(old_capacity);
    this->data = GROW_ARRAY(T, this->data, old_capacity, this->capacity_);
  }

  this->data[this->count] = item;
  this->count++;

  return this->count - 1;
}

template <typename T>
auto DynamicArray<T>::Append(T* items, u64 size) -> u64 {
  while (this->capacity_ < this->count + size) {
    u64 old_capacity = this->capacity_;
    this->capacity_ = GROW_CAPACITY(old_capacity);

    this->data = GROW_ARRAY(T, this->data, old_capacity, this->capacity_);
  }

  std::memcpy(&this->data[this->count], items, sizeof(T) * size);
  this->count += size;

  return this->count - size;
}

template <typename T>
auto DynamicArray<T>::Deinit() -> void {
  FREE_ARRAY(T, this->data, this->capacity_);
  this->Init();
}

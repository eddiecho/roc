#pragma once

#include <stdlib.h>
#include <string.h>

#include <utility>

#include "common.h"
#include "memory.h"

#define DEFAULT_ARENA_SIZE_BYTES 1024 * 1024

template <typename T>
class Arena {
  u32 capacity;
  u32 count;
  T* data;
  // @TODO(eddie) - unique_ptr
  Arena* next = nullptr;

 public:
  Arena() {
    using type = T;

    this->count = 0;
    this->capacity = DEFAULT_ARENA_SIZE_BYTES / sizeof(type);
    this->data = reinterpret_cast<T*>(malloc(DEFAULT_ARENA_SIZE_BYTES));
  };

  Arena(u32 size) {
    using type = T;

    this->count = 0;
    this->capacity = size;
    this->data = reinterpret_cast<T*>(malloc(size  * sizeof(type)));
  };

  ~Arena() {
    free(this->data);
    if (this->next != nullptr) {
      delete this->next;
    }
  };

  auto Push(T&& entry) -> T*;
  auto PushArray(T* entry, u32 len) -> T*;
  auto Position() const -> T*;
  auto AllocatedBytes() const -> u32;
  auto Clear() -> void;
  auto ClearEntries(u32 amount) -> void;
};

template <typename T>
auto Arena<T>::Push(T&& entry) -> T* {
  if (this->count == this->capacity) {
    if (this->next == nullptr) {
      // @FIXME(eddie) - does this work?
      this->next = new Arena<T>(this->capacity);
    }

    return this->next->Push(entry);
  } else {
    this->data[this->count] = entry;
    this->count++;

    return &this->data[this->count - 1];
  }
}

// @STDLIB
template <typename T>
auto Arena<T>::PushArray(T* entry, u32 len) -> T* {
  using type = T;

  if (this->capacity - this->count >= len) {
    memcpy(this->data + this->count, entry, len * sizeof(type));
    this->count += len;

    return &this->data[this->count - len];
  } else {
    if (this->next == nullptr) {
      u32 new_size = this->capacity < len
        ? len
        : this->capacity;
      this->next = new Arena<T>(new_size);
    }

    return this->next->PushArray(entry, len);
  }
}

template <typename T>
auto Arena<T>::Position() const -> T* {
  return this->data + (this->count);
}

template <typename T>
auto Arena<T>::AllocatedBytes() const -> u32 {
  using type = T;
  return this->count * sizeof(type);
}

template <typename T>
auto Arena<T>::Clear() -> void {
  this->count = 0;
}

template <typename T>
auto Arena<T>::ClearEntries(u32 amount) -> void {
  u32 real_amount = this->count < amount ? this->count : amount;
  this->count -= real_amount;
}

#pragma once

#include <stdlib.h>
#include <string.h>

#include <memory>
#include <type_traits>

#include "common.h"
#include "memory.h"

#define DEFAULT_ARENA_SIZE 128

template <typename T>
concept Nodeable = requires {
  T::next;
};

template <Nodeable T>
class Arena {
 public:
  Arena() {
    this->count = 0;
    this->capacity = DEFAULT_ARENA_SIZE;
    this->data = reinterpret_cast<T*>(malloc(DEFAULT_ARENA_SIZE * sizeof(T)));
  };

  Arena(u64 size) {
    this->count = 0;
    this->capacity = size;
    this->data = reinterpret_cast<T*>(malloc(size * sizeof(T)));
  };

  ~Arena() {
    free(this->data);
    if (this->next != nullptr) {
      delete this->next;
    }
  };

  fnc Alloc() -> u64;
  fnc Alloc(u64 len) -> u64;
  fnc Free(T* entry) -> void;
  fnc Clear() -> void;
  fnc AllocatedBytes() const -> u64;
  fnc Nth(u64 idx) -> T*;

 private:
  fnc Push() -> u64;

 private:
  u64 capacity;
  u64 count;
  T* data;
  Arena<T>* next = nullptr;
  T* first_free = nullptr;

};

template <Nodeable T>
fnc Arena<T>::Push() -> u64 {
  if (this->count == this->capacity) {
    if (this->next == nullptr) {
      // @FIXME(eddie) - does this work?
      this->next = new Arena<T>(this->capacity);
    }

    return this->capacity + this->next->Push();
  } else {
    this->count++;

    return this->count - 1;
  }
}

template <Nodeable T>
fnc Arena<T>::AllocatedBytes() const -> u64 {
  return this->count * sizeof(T);
}

template <Nodeable T>
fnc Arena<T>::Clear() -> void {
  this->count = 0;
}

template <Nodeable T>
fnc Arena<T>::Alloc() -> u64 {
  T* result = this->first_free;
  if (result != nullptr) {
    this->first_free = this->first_free->next;
    this->first_free->next = nullptr;

    return result - this->data;
  }

  return this->Push();
}

template <Nodeable T>
fnc Arena<T>::Free(T* entry) -> void {
  entry->next = this->first_free;
  this->first_free = entry;

  this->count--;
}

template <Nodeable T>
fnc Arena<T>::Nth(u64 idx) -> T* {
  if (idx > this->capacity) return this->next->Nth(idx - this->capacity);

  return &this->data[idx];
}

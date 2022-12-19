#pragma once

#include "../dynamic_array.h"

template <typename T>
struct Range {
  u32 min;
  T val;

  Range(u32 min, T val) : min(min), val(val) {}
};

// Array for holding contiguous intervals that map to some value
template <typename T>
class RangeArray : public DynamicArray<Range<T>> {
 public:
  auto Search(u32 val) -> const u32;

  auto operator[](std::size_t idx) -> Range<T>& {
    return this->data[idx];
  }
  const Range<T>& operator[](std::size_t idx) const {
    return this->data[idx];
  }

};

template <typename T>
// returns the index of the Range struct
auto RangeArray<T>::Search(u32 range) -> const u32 {
  if (this->count == 0) {
    return 0xFFFFFFFF;
  }

  if (this->count == 1) {
    return 0;
  }

  int lo = 0;
  int hi = this->count - 1;
  int mid;

  while (hi - lo > 1) {
    mid = (lo + hi) / 2;

    // cpp thinks because this is a pointer, its just normal subscript
    // cant overload subscript on pointer types zzzz
    if (range < (*this)[mid].min) {
      hi = mid;
    } else {
      lo = mid + 1;
    }
  }

  return range < ((*this)[hi].min) ? lo : hi;
}

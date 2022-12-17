#pragma once

#include "../dynamic_array.h"

template <typename T>
struct Range {
  u32 min;
  T val;
};

// Array for holding contiguous intervals that map to some value
template <typename T>
struct RangeArray : public DynamicArray<Range<T>> {
  auto Search(u32 val) -> const u32;
};

template <typename T>
// returns the index of the Range struct
auto RangeArray<T>::Search(u32 range) -> const u32 {
  if (this->count_ == 0) {
    return 0xFFFFFFFF;
  }

  if (this->count_ == 1) {
    return 0;
  }

  int lo = 0;
  int hi = this->count_ - 1;
  int mid;

  while (hi - lo > 1) {
    mid = (lo + hi) / 2;
    if (range < this->data_[mid].min) {
      hi = mid;
    } else {
      lo = mid + 1;
    }
  }

  return range < this->data_[hi].min ? lo : hi;
}

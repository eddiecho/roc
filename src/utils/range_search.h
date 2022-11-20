#pragma once

#include <algorithm>
#include <tuple>

#include "../dynamic_array.h"

template <typename T>
struct Range {
  u32 min;
  T val;
};

// Array for holding contiguous intervals that map to some value
template <typename T>
struct RangeArray : DynamicArray<Range<T>> {
  u32 search(u32 val);
};

template <typename T>
// returns the index of the Range struct
auto RangeArray<T>::search(u32 range) -> u32 {
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
    if (range < this->data[mid].min) {
      hi = mid;
    } else {
      lo = mid + 1;
    }
  }

  return range < this->data[hi].min ? lo : hi;
}

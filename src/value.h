#pragma once

#include <stdio.h>

#include "common.h"
#include "dynamic_array.h"

typedef double Value;

auto PrintValue(const Value val) -> void {
  printf("%g", val);
}

struct ConstData : DynamicArray<Value> {};

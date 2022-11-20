#pragma once

#include <stdio.h>

#include "common.h"
#include "dynamic_array.h"

typedef double Value;

void printValue(Value val) {
  printf("%g", val);
}

struct ConstData : DynamicArray<Value> {};

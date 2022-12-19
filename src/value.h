#pragma once

#include <stdio.h>

#include "common.h"
#include "dynamic_array.h"

enum class ValueType {
  Number,
  Boolean,
};

struct Value {
  ValueType type;
  union {
    bool boolean;
    f64 number;
  } as;

  Value() noexcept {
    this->type = ValueType::Number;
    // @FIXME(eddie) - maybe make this a sentinel
    this->as.number = 0;
  };

  Value(f64 num) noexcept {
    this->type = ValueType::Number;
    this->as.number = num;
  };

  auto Print() const -> const void;
};

struct ConstData : DynamicArray<Value> {};

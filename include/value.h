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

  Value(bool boolean) noexcept {
    this->type = ValueType::Boolean;
    this->as.boolean = boolean;
  }

  bool operator==(const Value other) {
    // @TODO(eddie) - type deduction
    if (this->type != other.type) return false;

    switch (this->type) {
      default:
        return false;
      case ValueType::Boolean: {
        return this->as.boolean == other.as.boolean;
      }
      case ValueType::Number: {
        return this->as.number == other.as.number;
      }
    }
  }

  auto Print() const -> const void;
};

struct ConstData : DynamicArray<Value> {};

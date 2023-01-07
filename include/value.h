#pragma once

#include <stdio.h>

#include "common.h"
#include "dynamic_array.h"
#include "object.h"

enum class ValueType {
  Number,
  Boolean,
  Object,
};

struct Value {
  ValueType type;
  union {
    bool boolean;
    f64 number;
    Object* object;
  } as;

  Value() noexcept {
    this->type = ValueType::Number;
    // @FIXME(eddie) - maybe make this a sentinel
    this->as.number = 0;
  }

  Value(f64 num) noexcept {
    this->type = ValueType::Number;
    this->as.number = num;
  }

  Value(bool boolean) noexcept {
    this->type = ValueType::Boolean;
    this->as.boolean = boolean;
  }

  Value(Object* object) noexcept {
    this->type = ValueType::Object;
    this->as.object = object;
  }

  auto operator==(const Value other) -> bool {
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
      case ValueType::Object: {
        return this->as.object == other.as.object;
      }
    }
  }

  auto constexpr inline IsObject() -> bool;
  auto Print() const -> const void;
};

struct ConstData : DynamicArray<Value> {};

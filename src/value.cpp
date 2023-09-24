#include "value.h"

#include "common.h"
#include "object.h"

auto Value::Print() const -> const void {
  switch (this->type) {
    default: {
      printf("Unknown type");
      return;
    }
    case ValueType::Boolean: {
      printf(this->as.boolean ? "true" : "false");
      return;
    }
    case ValueType::Number: {
      printf("%f", this->as.number);
      return;
    }
    case ValueType::Object: {
      printf("Object: ");
      this->as.object->Print();
      return;
    }
  }
}

auto Value::IsTruthy() const -> bool {
  switch (this->type) {
    default:
      return false;
    case ValueType::Boolean:
      return this->as.boolean;
    case ValueType::Number:
      return this->as.number != 0.0;
    case ValueType::Object:
      return this->as.object->IsTruthy();
  }
}

auto Value::IsObject() const -> bool {
  return this->type == ValueType::Object;
}

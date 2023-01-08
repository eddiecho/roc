#include "value.h"

#include "common.h"
#include "object.h"

fnc Value::Print() const -> const void {
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

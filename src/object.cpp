#include "object.h"

#include <stdio.h>

#include "common.h"
#include "memory.h"
#include "utils.h"
#include "value.h"

fnc Object::Print() -> void {
  switch(this->type) {
    default: {
      printf("Unknown object type\n");
      return;
    };
    case ObjectType::String: {
      static_cast<Object::String*>(this)->Print();
      return;
    };
  }
}

fnc Object::String::Print() -> void {
  printf("String: %s", this->as.string.start);
}

fnc Object::String::Init(u32 len, const char* start) -> void {
  this->type = ObjectType::String;
  this->hash = Utils::HashString(start, len);

  this->as.string.length = len;
  this->as.string.start = start;
}

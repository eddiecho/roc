#include "object.h"

#include <stdio.h>

#include "common.h"
#include "memory.h"
#include "value.h"

func Object::String::Print() -> void {
  printf("String: %s", this->start);
}

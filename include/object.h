#pragma once

#include "common.h"
#include "value.h"

enum class ObjectType {
  String,
};

struct Object {
  ObjectType type;
};

struct ObjectString : Object {
  u32 length;
  const char* start;

  ObjectString(u32 length, const char* start) :
    length(length), start(start) {};
};

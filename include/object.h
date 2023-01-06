#pragma once

#include <cstring>
#include <string>

#include "common.h"

enum class ObjectType {
  String,
};

struct Object {
  ObjectType type;

  struct String;

  func virtual Print() -> void = 0;

  func operator==(const Object* other) -> bool {
    if (this->type != other->type) return false;

    return this == other;
  }
};

struct Object::String : Object {
  u32 length;
  const char* start;

  String(u32 length, const char* start) noexcept {
    this->type = ObjectType::String;
    this->length = length;
    this->start = start;
  };

  String(std::string str) noexcept {
    this->type = ObjectType::String;
    this->length = str.length();
    this->start = str.c_str();
  };

  func Print() -> void override;

  // @STDLIB
  func operator==(const String* other) -> bool {
    if (this->length != other->length) return false;

    return std::memcmp(this->start, other->start, this->length) == 0;
  }
};

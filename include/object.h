#pragma once

#include <cstring>
#include <string>

#include "absl/hash/hash.h"

#include "common.h"
#include "utils.h"

enum class ObjectType {
  String,
};

class Object {
 public:
  class String;

  // @WTF - cant use auto or fnc here, because cpp doesnt let you
  bool virtual operator==(const Object* o) const = 0;

  fnc Print() -> void;

  template <typename H>
  friend H AbslHashValue(H h, const Object& s) {
    return H::combine(std::move(h), s.hash);
  }

 public:
  ObjectType type;
  Object* next = nullptr;

  u32 hash;

  union {
    struct {
      u32 length;
      const char* start;
    } string;
  } as;
};

class Object::String : public Object {
 public:
  String(u32 length, const char* start) noexcept {
    this->type = ObjectType::String;
    this->hash = Utils::HashString(start, length);

    this->as.string.length = length;
    this->as.string.start = start;
  };

  String(std::string str) noexcept {
    this->type = ObjectType::String;
    this->hash = Utils::HashString(str.c_str(), str.length());

    this->as.string.length = str.length();
    this->as.string.start = str.c_str();
  };

  // @STDLIB
  fnc operator==(const Object* o) const -> bool override {
    if (o->type != ObjectType::String) return false;

    auto other = static_cast<const Object::String*>(o);
    if (this->as.string.length != other->as.string.length) return false;

    return std::memcmp(this->as.string.start, other->as.string.start, this->as.string.length) == 0;
  }

  fnc operator==(const Object::String o) const -> bool {
    if (this->hash != o.hash) return false;
    if (this->as.string.length != o.as.string.length) return false;

    return std::memcmp(this->as.string.start, o.as.string.start, this->as.string.length) == 0;
  }

  fnc Print() -> void;
  fnc Init(u32 length, const char* start) -> void;
};

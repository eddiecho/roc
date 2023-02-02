#pragma once

#include <cstring>
#include <string>

#include "absl/hash/hash.h"

#include "chunk.h"
#include "common.h"
#include "utils.h"

enum class ObjectType {
  String,
};

class Object {
 public:
  class String;
  class Function;

  // @WTF - cant use auto or fnc here, because cpp doesnt let you
  bool virtual operator==(const Object* o) const = 0;

  fnc Print() -> void;
  fnc IsTruthy() -> bool;

  template <typename H>
  friend H AbslHashValue(H h, const Object& s);

 public:
  ObjectType type;
  Object* next = nullptr;

 private:
  u32 hash;

  union {
    struct {
      u32 length;
      const char* start;
    } string;
    struct {
      u32 arity;
      Chunk* chunk;
      const char* name;
      u32 name_len;
    } function;
  } as;
};

class Object::String : public Object {
 public:
  String() noexcept;
  String(u32 length, const char* start) noexcept;
  String(std::string str) noexcept;
  String(Object::String& str) noexcept;
  String(const Object::String&& str) noexcept;

  template <typename H>
  friend H AbslHashValue(H h, const Object& s) {
    return H::combine(std::move(h),
                      s.hash,
                      std::string(s.as.string.start, s.as.string.length));
  }

  operator std::string() const {
    std::string ret{this->as.string.start, this->as.string.length};

    return ret;
  }

  // @STDLIB
  fnc operator==(const Object* o) const -> bool override {
    if (o->type != ObjectType::String) return false;

    auto other = static_cast<const Object::String*>(o);
    if (this->as.string.length != other->as.string.length) return false;

    return std::memcmp(this->as.string.start, other->as.string.start, this->as.string.length) == 0;
  }

  fnc operator==(Object::String o) const -> bool {
    if (this->hash != o.hash) return false;
    if (this->as.string.length != o.as.string.length) return false;

    return std::memcmp(this->as.string.start, o.as.string.start, this->as.string.length) == 0;
  }

  fnc operator==(std::string str) const -> bool {
    if (this->as.string.length != str.length()) return false;

    return std::memcmp(this->as.string.start, str.c_str(), this->as.string.length) == 0;
  }

  fnc Print() -> void;
  fnc IsTruthy() -> bool;
  fnc Init(u32 length, const char* start) -> void;
  fnc Init(const Object::String&& str) -> void;
};

class Object::Function : public Object {

};


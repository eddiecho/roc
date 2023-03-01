#pragma once

#include <cstring>
#include <string>

#include "absl/hash/hash.h"

#include "chunk.h"
#include "common.h"
#include "utils.h"

enum class ObjectType {
  String,
  Function,
};
#define OBJECT_TYPE_COUNT 2

class Object {
 public:
  class String;
  struct StringData {};

  class Function;
  struct FunctionData {
    u32 arity;
    Chunk chunk;

    fnc Init(Chunk chunk) -> void {
      this->arity = 0;
      this->chunk = chunk;
    }
  };

  bool operator==(const Object* o) const {
    return this->type == o->type;
  }

  fnc Print() -> void;
  fnc IsTruthy() -> bool;

  template <typename H>
  friend H AbslHashValue(H h, const Object& s) {
    return H::combine(std::move(h), static_cast<u32>(s.type), std::string(s.name, s.name_len));
  }

 public:
  ObjectType type;

  // this gets overloaded for two uses
  // 1. free lists in GlobalPools to find the next free memory slot
  // 2. recursive and enclosing functions pointing to their parent
  Object* next = nullptr;

  u32 name_len;
  const char* name;

  union Data {
    StringData string;
    FunctionData function;

    Data() { memset(this, 0, sizeof(Data)); }
    ~Data() {}
  } as;
};

class Object::String : public Object {
 public:
  String() noexcept;
  String(u32 name_len, const char* name) noexcept;
  String(std::string str) noexcept;
  String(Object::String& str) noexcept;
  String(const Object::String&& str) noexcept;

  operator std::string() const {
    std::string ret{this->name, this->name_len};

    return ret;
  }

  fnc operator==(const Object* o) const -> bool {
    if (o->type != ObjectType::String) return false;

    auto other = static_cast<const Object::String*>(o);
    if (this->name_len != other->name_len) return false;

    return std::memcmp(this->name, other->name, this->name_len) == 0;
  }

  fnc operator==(Object::String o) const -> bool {
    if (this->name_len != o.name_len) return false;

    return std::memcmp(this->name, o.name, this->name_len) == 0;
  }

  fnc operator==(std::string str) const -> bool {
    if (this->name_len != str.length()) return false;

    return std::memcmp(this->name, str.c_str(), this->name_len) == 0;
  }

  fnc Print() -> void;
  fnc IsTruthy() -> bool;
  fnc Init(u32 name_len, const char* name) -> void;
  fnc Init(const Object::String&& str) -> void;
};

class Object::Function : public Object {
 public:
  Function() noexcept;

  fnc operator==(const Object* o) const -> bool {
    if (o->type != ObjectType::Function) return false;
    if (o->as.function.arity != this->as.function.arity) return false;
    if (o->name_len != this->name_len) return false;

    return std::memcmp(this->name, o->name, this->name_len) == 0;
  }

  fnc Print() -> void;
};


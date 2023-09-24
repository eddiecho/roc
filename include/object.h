#pragma once

#include <cstring>
#include <string>

#include "chunk.h"
#include "common.h"
#include "utils.h"

enum class ObjectType {
  String,
  Function,
  Closure,
  Upvalue,
};

class Object {
 public:
  class String;
  struct StringData {};

  class Upvalue;
  struct UpvalueData {
    Value* location;
    Object::Upvalue* next;
    Value closed_value;
  };

  class Function;
  struct FunctionData {
    u32 arity;
    u32 upvalue_count;
    Chunk* chunk;
  };

  class Closure;
  struct ClosureData : FunctionData {
    Object::Upvalue** upvalues;
  };

  auto operator==(const Object* o) const -> bool { return this->type == o->type; }

  auto Print() const -> void;
  auto IsTruthy() const -> const bool;

 public:
  Object() = default;

  ObjectType type;

  // used for free lists in GlobalPools to find the next free memory slot
  Object* next = nullptr;

  u32 name_len;
  const char* name;

  union Data {
    StringData string;
    FunctionData function;
    ClosureData closure;
    UpvalueData upvalue;

    Data() { string = {}; }
    ~Data() {}
  } as;
};

class Object::String : public Object {
 public:
  String() noexcept;
  String(u32 name_len, const char* name) noexcept;
  explicit String(std::string_view str) noexcept;
  String(Object::String& str) noexcept;
  String(const Object::String&& str) noexcept;

  explicit operator std::string_view() const {
    const std::string_view ret{this->name, this->name_len};
    return ret;
  }

  auto operator==(const Object* o) const -> bool {
    if (o->type != ObjectType::String) return false;

    const auto* other = static_cast<const Object::String*>(o);
    if (this->name_len != other->name_len) return false;

    return std::memcmp(this->name, other->name, this->name_len) == 0;
  }

  auto operator==(Object::String& o) const -> bool {
    if (this->name_len != o.name_len) return false;

    return std::memcmp(this->name, o.name, this->name_len) == 0;
  }

  auto Print() const -> void;
  auto IsTruthy() -> bool;
  auto Init(u32 name_len, const char* name) -> void;
  auto Init(const Object::String&& str) -> void;
};

class Object::Function : public Object {
 public:
  Function() noexcept;

  auto operator==(const Object* o) const -> bool {
    if (o->type != ObjectType::Function) return false;
    if (o->as.function.arity != this->as.function.arity) return false;
    if (o->name_len != this->name_len) return false;

    return std::memcmp(this->name, o->name, this->name_len) == 0;
  }

  auto Init(Chunk* chunk, u32 name_len, const char* name) -> void;
  auto Print() const -> void;
  auto inline Unwrap() -> Object::FunctionData;
};

class Object::Closure : public Object {
 public:
  Closure() noexcept;

  auto operator==(const Object* o) const -> bool {
    if (o->type != ObjectType::Closure) return false;

    if (o->as.closure.arity != this->as.closure.arity) return false;
    if (o->name_len != this->name_len) return false;
    if (o->as.closure.upvalue_count != this->as.closure.upvalue_count) return false;

    return std::memcmp(this->name, o->name, this->name_len) == 0;
  }

  auto Init(const Object* function) -> void;
  auto Init(const Object::Function* function) -> void;
  auto Deinit() -> void;
  auto Print() const -> void;
};

class Object::Upvalue : public Object {
 public:
  Upvalue() noexcept;

  auto operator==(const Object* o) const -> bool {
    if (o->type != ObjectType::Upvalue) return false;

    return this->as.upvalue.location == o->as.upvalue.location;
  }

  auto Init(Value* location) -> void;
  auto Print() const -> void;
};

#include "object.h"

#include <stdio.h>

#include <string>

#include "common.h"
#include "memory.h"
#include "utils.h"

fnc Object::Print() const->void {
  switch (this->type) {
    default: {
      printf("Unknown object type\n");
      return;
    };
    case ObjectType::String: {
      static_cast<const Object::String*>(this)->Print();
      return;
    };
    case ObjectType::Function: {
      static_cast<const Object::Function*>(this)->Print();
      return;
    }
    case ObjectType::Closure: {
      static_cast<const Object::Closure*>(this)->Print();
      return;
    }
    case ObjectType::Upvalue: {
      static_cast<const Object::Upvalue*>(this)->Print();
      return;
    }
  }
}

fnc Object::IsTruthy() const -> const bool {
  switch (this->type) {
    default:
      return true;
    case ObjectType::String: {
      return static_cast<const Object::String*>(this)->name_len > 0;
    }
  }
}

Object::String::String() noexcept {
  this->type = ObjectType::String;
  this->name_len = 0;
  this->name = nullptr;
}

Object::String::String(u32 name_len, const char* name) noexcept {
  this->type = ObjectType::String;
  this->name_len = name_len;
  this->name = name;
}

Object::String::String(std::string_view str) noexcept {
  this->type = ObjectType::String;

  this->name_len = str.length();
  this->name = str.data();
}

Object::String::String(Object::String& str) noexcept {
  this->type = ObjectType::String;
  this->name_len = str.name_len;
  this->name = str.name;
}

Object::String::String(const Object::String&& str) noexcept {
  this->type = ObjectType::String;
  this->name_len = str.name_len;
  this->name = str.name;
}

fnc Object::String::Print() const -> void {
  printf("String: %s", this->name);
}

fnc Object::String::Init(u32 len, const char* name) -> void {
  this->type = ObjectType::String;
  this->name_len = len;
  this->name = name;
}

fnc Object::String::Init(const Object::String&& str) -> void {
  this->type = ObjectType::String;
  this->name_len = str.name_len;
  this->name = str.name;
}

Object::Function::Function() noexcept {
  this->type = ObjectType::Function;
  this->as.function.arity = 0;
  this->name = "";
  this->name_len = 0;
}

fnc Object::Function::Print() const -> void {
  printf("Function: %s", this->name);
}

fnc inline Object::Function::Unwrap() -> Object::FunctionData {
  return this->as.function;
}

fnc Object::Function::Init(Chunk* chunk, u32 name_len, const char* name) -> void {
  this->type = ObjectType::Function;
  this->as.function.arity = 0;
  this->as.function.upvalue_count = 0;
  this->as.function.chunk = chunk;
  this->next = nullptr;

  this->name_len = name_len;
  this->name = name;
}

Object::Closure::Closure() noexcept {
  this->type = ObjectType::Closure;
  this->name_len = 0;
  this->name = 0;
  this->as.closure = {};
}

fnc Object::Closure::Init(const Object::Function* function) -> void {
  this->type = ObjectType::Closure;
  this->name_len = function->name_len;
  this->name = function->name;
  this->as.closure.upvalue_count = function->as.function.upvalue_count;
  this->as.closure.arity = function->as.function.arity;
  this->as.closure.chunk = function->as.function.chunk;

  auto upvalues_count = function->as.function.upvalue_count;
  this->as.closure.upvalue_count = upvalues_count;
  this->as.closure.upvalues = reinterpret_cast<Object::Upvalue**>(
      malloc(sizeof(Object::Upvalue*) * upvalues_count));
}

fnc Object::Closure::Init(const Object* obj) -> void {
  Assert(obj->type == ObjectType::Function);
  auto function = static_cast<const Object::Function*>(obj);

  this->Init(obj);
}

fnc Object::Closure::Deinit() -> void {
  free(this->as.closure.upvalues);
}

fnc Object::Closure::Print() const -> void {
  printf("Function: %s", this->name);
}

Object::Upvalue::Upvalue() noexcept {
  this->type = ObjectType::Upvalue;
  this->as.upvalue.location = nullptr;
  this->as.upvalue.closed_value = {};
  this->as.upvalue.next = nullptr;
}

fnc Object::Upvalue::Init(Value* value) -> void {
  this->type = ObjectType::Upvalue;
  this->as.upvalue.location = value;
  this->as.upvalue.closed_value = {};
  this->as.upvalue.next = nullptr;
}

fnc Object::Upvalue::Print() const -> void {
  printf("upvalue");
}

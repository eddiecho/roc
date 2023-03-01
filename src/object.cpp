#include "object.h"

#include <stdio.h>
#include <string>

#include "common.h"
#include "memory.h"
#include "utils.h"

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
    case ObjectType::Function: {
      static_cast<Object::Function*>(this)->Print();
      return;
    }
  }
}

fnc Object::IsTruthy() -> bool {
  switch (this->type) {
    default:
      return true;
    case ObjectType::String: {
      return static_cast<Object::String*>(this)->name_len > 0;
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

Object::String::String(std::string str) noexcept {
  this->type = ObjectType::String;

  this->name_len = str.length();
  this->name = str.c_str();
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

fnc Object::String::Print() -> void {
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

fnc Object::Function::Print() -> void {
  printf("Function: %s", this->name);
}

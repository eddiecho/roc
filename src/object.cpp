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
  }
}

fnc Object::IsTruthy() -> bool {
  switch (this->type) {
    default:
      return true;
    case ObjectType::String: {
      return static_cast<Object::String*>(this)->as.string.length > 0;
    }
  }
}

Object::String::String() noexcept {
  this->type = ObjectType::String;
  this->hash = 0;
  this->as.string.length = 0;
  this->as.string.start = nullptr;
}

Object::String::String(u32 length, const char* start) noexcept {
  this->type = ObjectType::String;
  this->hash = Utils::HashString(start, length);

  this->as.string.length = length;
  this->as.string.start = start;
}

Object::String::String(std::string str) noexcept {
  this->type = ObjectType::String;
  this->hash = Utils::HashString(str.c_str(), str.length());

  this->as.string.length = str.length();
  this->as.string.start = str.c_str();
}

Object::String::String(Object::String& str) noexcept {
  this->type = ObjectType::String;
  this->hash = str.hash;

  this->as.string.length = str.as.string.length;
  this->as.string.start = str.as.string.start;
}

Object::String::String(const Object::String&& str) noexcept {
  this->type = ObjectType::String;
  this->hash = str.hash;

  this->as.string.length = str.as.string.length;
  this->as.string.start = str.as.string.start;
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

fnc Object::String::Init(const Object::String&& str) -> void {
  this->type = ObjectType::String;
  this->hash = str.hash;

  this->as.string.length = str.as.string.length;
  this->as.string.start = str.as.string.start;
}


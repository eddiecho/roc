#pragma once

#include <cstring>
#include <string>

#include "common.h"

#define DEFAULT_INIT_STR_POOL_SIZE 1024 * 1024

enum class ObjectType {
  String,
};

class Object {
 public:

  class String;

  ObjectType type;
  Object* next = nullptr;

  func virtual Print() -> void = 0;
  // @WTF - cant use auto or func here, because cpp doesnt let you
  bool virtual operator==(const Object* o) = 0;
};

class Object::String : public Object {
 public:
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

  // @STDLIB
  func operator==(const Object* o) -> bool override {
    if (o->type != ObjectType::String) return false;

    auto other = static_cast<const String*>(o);

    if (this->length != other->length) return false;

    return std::memcmp(this->start, other->start, this->length) == 0;
  }

  func Print() -> void override;

 public:
  u32 length;

 private:
  const char* start;
};

// @TODO - investigate a free list style pool arena implementation
// kinda need to be able to know how big an object is
class ObjectArena {
 public:
  func Init();
  func Deinit();

  func Push(u64 size, void* obj);
  func Clear();

 private:
  void* area;
};

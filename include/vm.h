#pragma once

#include <stdarg.h>
#include <stdio.h>

#include "absl/container/flat_hash_set.h"

#include "arena.h"
#include "chunk.h"
#include "common.h"
#include "dynamic_array.h"
#include "object.h"
#include "value.h"

#define VM_INTERPRET_ERRORS \
  X(Success)                \
  X(CompileError)           \
  X(RuntimeError)

enum class InterpretError {
#define X(ID) ID,
  VM_INTERPRET_ERRORS
#undef X
};

fnc static ErrorToString(InterpretError err) -> const char* {
  switch (err) {
#define X(ID)              \
  case InterpretError::ID: \
    return #ID;
    VM_INTERPRET_ERRORS
#undef X

    default: {
      return "Success";
    }
  }
}

#define VM_STACK_MAX 256
class VirtualMachine {
 public:
  fnc Init() -> void;
  fnc Deinit() -> void;
  fnc Interpret(Chunk* chunk, DynamicArray<char>* string_pool) -> InterpretError;
  fnc RuntimeError(const char* msg, ...) -> void;
  fnc Reset() -> void;
  fnc Peek() const -> Value;
  fnc Peek(int dist) const -> Value;

 private:
  Chunk* chunk = nullptr;
  u8* inst_ptr = nullptr;
  Value stack[VM_STACK_MAX];
  Value* stack_top;

  // this stores the actual string characters
  DynamicArray<char>* string_pool = nullptr;
  // this is used for interning and lookup
  absl::flat_hash_set<Object::String> string_table;
  Arena<Object>* object_pool = new Arena<Object>();

  fnc Push(Value value) -> void;
  fnc Pop() -> Value;
};

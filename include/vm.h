#pragma once

#include <stdarg.h>
#include <stdio.h>

#include "chunk.h"
#include "dynamic_array.h"
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

func static ErrorToString(InterpretError err) -> const char* {
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
  func Init() -> void;
  func Deinit() -> void;
  func Interpret(Chunk* chunk, DynamicArray<char>* string_pool) -> InterpretError;
  func RuntimeError(const char* msg, ...) -> void;
  func Reset() -> void;
  func Peek() const -> Value;
  func Peek(int dist) const -> Value;

 private:
  Chunk* chunk = nullptr;
  u8* inst_ptr = nullptr;
  Value stack[VM_STACK_MAX];
  Value* stack_top;
  DynamicArray<char>* string_pool = nullptr;

  func Push(Value value) -> void;
  func Pop() -> Value;
};

#pragma once

#include <stdarg.h>
#include <stdio.h>

#include "arena.h"
#include "chunk.h"
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

auto static ErrorToString(InterpretError err) -> const char* {
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
  auto Init() -> void;
  auto Deinit() -> void;
  auto Interpret(Chunk* chunk) -> InterpretError;
  auto RuntimeError(const char* msg, ...) -> void;
  auto Reset() -> void;
  auto Peek() const -> Value;
  auto Peek(int dist) const -> Value;

 private:
  Chunk* chunk = nullptr;
  u8* inst_ptr = nullptr;
  Value stack[VM_STACK_MAX];
  Value* stack_top;
  Arena<char>* string_pool;

  auto Push(Value value) -> void;
  auto Pop() -> Value;
};

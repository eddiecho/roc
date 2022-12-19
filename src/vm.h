#pragma once

#include <stdio.h>

#include "chunk.h"
#include "value.cpp"

#define VM_INTERPRET_ERRORS \
  X(Success) \
  X(CompileError) \
  X(RuntimeError)

enum class InterpretError {
#define X(ID) ID,
  VM_INTERPRET_ERRORS
#undef X
};

auto static errorToString(InterpretError err) -> const char* {
  switch (err) {
#define X(ID) case InterpretError::ID: return #ID;
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
  auto Interpret(Chunk *chunk) -> InterpretError;

 private:
  Chunk* chunk = nullptr;
  u8* instructionPointer = 0;
  Value stack[VM_STACK_MAX];
  Value* stackTop;

  auto Push(Value value) -> void;
  auto Pop() -> Value;
};

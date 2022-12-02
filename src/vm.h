#pragma once

#include <stdio.h>

#include "chunk.h"
#include "value.h"

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
struct VirtualMachine {
  Chunk* chunk = nullptr;
  u8* instructionPointer = 0;
  Value stack[VM_STACK_MAX];
  Value* stackTop;

  auto init() -> void;
  auto deinit() -> void;
  auto interpret(Chunk *chunk) -> InterpretError;
  auto interpret(const char* src) -> InterpretError;
  auto push(Value value) -> void;
  auto pop() -> Value;
};

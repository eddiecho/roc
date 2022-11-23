#pragma once

#include <stdio.h>

#include "chunk.h"
#include "value.h"

enum class InterpretError {
  Success,
  CompileError,
  RuntimeError,
};

auto static printInterpretError(InterpretError err) -> void {
  switch (err) {
    case InterpretError::CompileError: {
      printf("InterpretError CompileError");
      break;
    }
    case InterpretError::RuntimeError: {
      printf("InterpretError RuntimeError");
      break;
    }
    default: {
      printf("InterpretError Success");
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
  auto push(Value value) -> void;
  auto pop() -> Value;
};

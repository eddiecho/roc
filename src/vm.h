#pragma once

#include <stdio.h>

#include "chunk.h"

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

struct VirtualMachine {
  Chunk* chunk = nullptr;
  u8* instructionPointer = 0;

  auto init() -> void;
  auto deinit() -> void;
  auto interpret(Chunk *chunk) -> InterpretError;
};

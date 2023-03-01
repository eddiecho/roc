#pragma once

#include <stdarg.h>
#include <stdio.h>

#include "absl/container/flat_hash_set.h"

#include "arena.h"
#include "chunk.h"
#include "common.h"
#include "dynamic_array.h"
#include "object.h"
#include "string_pool.h"
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

#define VM_STACK_MAX 128
#define VM_LOCAL_MAX VM_STACK_MAX * 4

struct StackFrame {
  Object::Function* function;
  u8* inst_ptr;
  Value* locals;
};

class VirtualMachine {
 public:
  fnc Init() -> void;
  fnc Deinit() -> void;
  fnc Interpret(
    Object::Function* func,
    StringPool* string_pool,
    Arena<Object>* object_pool
  ) -> InterpretError;

  fnc RuntimeError(const char* msg, ...) -> InterpretError;
  fnc Peek() const -> Value;
  fnc Peek(int dist) const -> Value;

 private:
  fnc Push(Value value) -> void;
  fnc Pop() -> Value;

 private:
  StackFrame frames[VM_STACK_MAX];
  u32 frame_count = 1;

  Value stack[VM_LOCAL_MAX];
  Value* stack_top;

  // @NOTE(eddie) - the string_pool manages its own Objects for strings
  StringPool* string_pool = nullptr;
  Arena<Object>* object_pool = nullptr;
};

#pragma once

#include <cstdarg>
#include <cstdio>

#include "absl/container/flat_hash_set.h"
#include "arena.h"
#include "chunk.h"
#include "common.h"
#include "dynamic_array.h"
#include "object.h"
#include "string_pool.h"
#include "utils.h"
#include "value.h"

#define VM_INTERPRET_ERRORS \
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

#define VM_STACK_MAX 128
#define VM_LOCAL_MAX (VM_STACK_MAX * 4)

enum class FrameType {
  Closure,
  Function,
};

// @TODO(eddie) - extract out instruction pointer, and put it
// directly into VirtualMachine to remove some indirection
struct StackFrame {
  FrameType type;

  union {
    Object::Closure* closure;
    Object::Function* function;
  };

  Chunk* chunk = nullptr;
  u8* inst_ptr;
  Value* locals;
};

using InterpretResult = Result<Value, InterpretError>;

class VirtualMachine {
 public:
  auto Init() -> void;
  auto Deinit() -> void;
  auto Interpret(Object* func, StringPool* string_pool, Arena<Object>* object_pool) -> InterpretResult;

  auto RuntimeError(const char* msg, ...) -> InterpretError;
  auto Peek() const -> Value;
  auto Peek(int dist) const -> Value;

 private:
  auto Push(Value value) -> void;
  auto Pop() -> Value;
  auto Invoke(Object::Closure* closure, u32 argc) -> Result<size_t, InterpretError>;
  auto Invoke(Object::Function* closure, u32 argc) -> Result<size_t, InterpretError>;
  auto CaptureUpvalue(Value* local) -> Object::Upvalue*;
  auto CloseUpvalues(Value* local) -> void;

 private:
  StackFrame frames[VM_STACK_MAX];
  u32 frame_count = 0;

  Value stack[VM_LOCAL_MAX];
  Value* stack_top;

  Object::Upvalue* open_upvalues = nullptr;

  // @NOTE(eddie) - the string_pool manages its own Objects for strings
  StringPool* string_pool = nullptr;
  Arena<Object>* object_pool = nullptr;
};

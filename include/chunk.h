#pragma once

#include "arena.h"
#include "common.h"
#include "dynamic_array.h"
#include "range_search.h"
#include "value.h"

enum class OpCode : u8 {
  Constant,  // 0
  ConstantLong,
  Add,
  Subtract,
  Multiply,
  Divide,
  Negate,
  Return,
  ReturnVoid,
  True,
  False,
  Not,  // 10
  Equality,
  Greater,
  Less,
  String,
  Pop,
  SetGlobal,
  GetGlobal,
  SetLocal,
  GetLocal,
  SetUpvalue,  // 20
  GetUpvalue,
  Jump,
  JumpFalse,
  JumpTrue,
  Loop,
  Invoke,
  Closure,
  CloseUpvalue,
};

using Bytecode = DynamicArray<u8>;
using LocalVariables = DynamicArray<Value>;
class Compiler;
class CompilerEngine;
class VirtualMachine;

class Chunk {
 public:
  friend class ChunkManager;

  Chunk() noexcept;

  friend Compiler;
  friend CompilerEngine;
  friend VirtualMachine;

  auto Init() -> void;
  auto Deinit() -> void;
  auto Disassemble() const -> void;
  auto AddInstruction(u8 byte, u64 line) -> u64;
  auto AddInstruction(u8* bytes, u64 count, u64 line) -> u64;
  auto AddLine(u64 line) -> void;
  auto AddLocal(Value val, u64 line) -> u64;
  auto Count() const -> u64;
  auto BaseInstructionPointer() const -> u8*;

 private:
  auto PrintAtOffset(int offset) const -> const int;
  auto SimpleInstruction(const char* name, int offset) const -> int;
  auto ConstantInstruction(int offset) const -> int;
  auto ConstantLongInstruction(int offset) const -> int;
  auto ByteInstruction(const char* name, int offset) const -> int;
  auto JumpInstruction(const char* name, int sign, int offset) const -> int;
  auto GlobalInstruction(const char* name, int offset) const -> int;

 private:
  Bytecode bytecode;
  LocalVariables locals;
  RangeArray<u64> lines;
};

class ChunkManager {
 public:
  auto Alloc() -> Chunk*;

 private:
  u64 count = 0;
  u64 capacity = 0;
  // @TODO(eddie) - investigate using Arena for this
  // Chunk needs a next field in that case
  Chunk* chunks = nullptr;
};

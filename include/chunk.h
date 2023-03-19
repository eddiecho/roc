#pragma once

#include "common.h"
#include "dynamic_array.h"
#include "range_search.h"
#include "value.h"

enum class OpCode : u8 {
  Constant,
  ConstantLong,
  Add,
  Subtract,
  Multiply,
  Divide,
  Negate,
  Return,
  True,
  False,
  Not,
  Equality,
  Greater,
  Less,
  String,
  Pop,
  SetGlobal,
  GetGlobal,
  SetLocal,
  GetLocal,
  SetUpvalue,
  GetUpvalue,
  Jump,
  JumpFalse,
  JumpTrue,
  Loop,
  Invoke,
  Closure,
};

using Bytecode = DynamicArray<u8>;
using LocalVariables = DynamicArray<Value>;
class Compiler;
class VirtualMachine;

class Chunk {
 public:
  Chunk() noexcept;

  friend Compiler;
  friend VirtualMachine;

  fnc Disassemble() const -> const void;
  fnc AddInstruction(u8 byte, u64 line) -> u64;
  fnc AddInstruction(u8* bytes, u64 count, u64 line) -> u64;
  fnc AddLine(u64 line) -> void;
  fnc AddLocal(Value val, u64 line) -> u64;
  fnc Count() const -> u64;
  fnc BaseInstructionPointer() const -> u8*;
  fnc Deinit() -> void;

 private:
  fnc PrintAtOffset(int offset) const -> const int;
  fnc SimpleInstruction(const char* name, int offset) const -> int;
  fnc ConstantInstruction(int offset) const -> int;
  fnc ConstantLongInstruction(int offset) const -> int;
  fnc ByteInstruction(const char* name, int offset) const -> int;
  fnc JumpInstruction(const char* name, int sign, int offset) const -> int;
  fnc GlobalInstruction(const char* name, int offset) const -> int;

 private:
  Bytecode bytecode;
  LocalVariables constants;
  RangeArray<u64> lines;
};

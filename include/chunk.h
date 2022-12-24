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
};

class VirtualMachine;

class Chunk : public DynamicArray<u8> {
 public:
  Chunk() noexcept;
  ~Chunk() noexcept;

  friend VirtualMachine;

  auto Disassemble() const -> const void;
  auto AddChunk(u8 byte, u32 line) -> void;
  auto AddLine(u32 line) -> void;
  auto AddConstant(Value val, u32 line) -> void;

 private:
  auto PrintAtOffset(int offset) const -> const int;
  auto SimpleInstruction(const char* name, int offset) const -> int;
  auto ConstantInstruction(int offset) const -> int;
  auto ConstantLongInstruction(int offset) const -> int;

 private:
  ConstData constants;
  RangeArray<u32> lines;
};

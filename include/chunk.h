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

  func Disassemble() const -> const void;
  func AddChunk(u8 byte, u32 line) -> void;
  func AddLine(u32 line) -> void;
  func AddConstant(Value val, u32 line) -> void;

 private:
  func PrintAtOffset(int offset) const -> const int;
  func SimpleInstruction(const char* name, int offset) const -> int;
  func ConstantInstruction(int offset) const -> int;
  func ConstantLongInstruction(int offset) const -> int;

 private:
  ConstData constants;
  RangeArray<u32> lines;
};

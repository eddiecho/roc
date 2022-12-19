#pragma once

#include "common.h"
#include "dynamic_array.h"
#include "utils/range_search.h"
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
};

struct Chunk : public DynamicArray<u8> {
  Chunk() noexcept;
  ~Chunk() noexcept;

  auto Disassemble() const -> const void;
  auto PrintAtOffset(int offset) const -> const int;
  auto AddChunk(u8 byte, u32 line) -> void;
  auto AddLine(u32 line) -> void;
  auto AddConstant(Value val, u32 line) -> void;

 public:
  ConstData constants;
  RangeArray<u32> lines;

};

#pragma once

#include "common.h"
#include "dynamic_array.h"
#include "utils/range_search.h"
#include "value.h"

enum OpCode {
  Constant,
  ConstantLong,
  Add,
  Subtract,
  Multiply,
  Divide,
  Negate,
  Return,
};

struct Chunk : DynamicArray<u8> {
  ConstData constants;
  RangeArray<u32> lines;

  Chunk() noexcept;
  ~Chunk();

  auto disassemble() -> void;
  auto printAtOffset(int offset) -> int;
  auto addChunk(u8 byte, u32 line) -> void;
  auto addLine(u32 line) -> void;
  auto addConstant(Value val, u32 line) -> void;
};

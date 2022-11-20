#pragma once

#include "common.h"
#include "dynamic_array.h"
#include "utils/range_search.h"
#include "value.h"

enum OpCode {
  CONSTANT,
  CONSTANT_LONG,
  RETURN,
};

struct Chunk : DynamicArray<u8> {
  ConstData constants;
  RangeArray<u32> lines;

  Chunk() noexcept;
  ~Chunk();

  void disassemble();
  int printAtOffset(int offset);
  void addChunk(u8 byte, u32 line);
  void addLine(u32 line);
  void addConstant(Value val, u32 line);
};

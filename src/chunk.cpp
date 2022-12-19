#pragma once

#include <stdio.h>

#include "chunk.h"
#include "value.h"

Chunk::Chunk() noexcept {
  this->Init();
  this->constants.Init();
  this->lines.Init();
}

Chunk::~Chunk() noexcept {
  this->lines.Deinit();
  this->constants.Deinit();
  this->Deinit();
}

auto Chunk::AddLine(u32 line) -> void {
  if (this->lines.count == 0) {
    this->lines.Append({this->count, line});
  } else {
    Range<u32> prev = this->lines[this->lines.count - 1];
    if (prev.val != line) {
      this->lines.Append({this->count, line});
    }
  }
}

auto Chunk::AddChunk(u8 byte, u32 line) -> void {
  this->AddLine(line);
  this->Append(byte);
}

#define SMALL_CONST_POOL_SIZE 256
auto Chunk::AddConstant(Value val, u32 line) -> void {
  this->AddLine(line);

  if (this->constants.count < SMALL_CONST_POOL_SIZE) {
    this->Append(static_cast<u8>(OpCode::Constant));
    this->Append((u8)this->constants.count);
  } else {
    this->Append(static_cast<u8>(OpCode::ConstantLong));

    u32 count = this->constants.count;
    this->Append(count >> 16);
    this->Append(count >> 8);
    this->Append(count);
  }

  this->constants.Append(val);
}

auto static SimpleInstruction(const char* name, int offset) -> int {
  printf("%s\n", name);
  return offset + 1;
}

auto static ConstantInstruction(const Chunk* chunk, int offset) -> int {
  u8 const_idx = (*chunk)[offset + 1];
  printf("%-16s %04d %04d %04d ' ", "OP_CONSTANT", 0, 0, const_idx);
  chunk->constants[const_idx].Print();
  printf("\n");

  return offset + 2;
}

auto static ConstantLongInstruction(const Chunk* chunk, int offset) -> int {
  u8 one = (*chunk)[offset + 1];
  u8 two = (*chunk)[offset + 2];
  u8 thr = (*chunk)[offset + 3];
  printf("%-16s %04d %04d %04d ' ", "OP_CONSTANT_LONG", one, two, thr);

  u32 idx = 0;
  idx |= (one << 16);
  idx |= (two << 8);
  idx |= (thr);

  chunk->constants[idx].Print();
  printf("\n");

  return offset + 4;
}

auto Chunk::PrintAtOffset(int offset) const -> const int {
  printf("%04d ", offset);

  u32 line_idx = this->lines.Search(offset);
  u32 line = this->lines[line_idx].val;
  printf("%4d ", line);

  u8 byte = (*this)[offset];
  OpCode instruction = static_cast<OpCode>(byte);

  switch (instruction) {
    case OpCode::Constant: {
      return ConstantInstruction(this, offset);
    }
    case OpCode::ConstantLong: {
      return ConstantLongInstruction(this, offset);
    }
    case OpCode::Negate: {
      return SimpleInstruction("OP_NEGATE", offset);
    }
    case OpCode::Add: {
      return SimpleInstruction("OP_ADD", offset);
    }
    case OpCode::Subtract: {
      return SimpleInstruction("OP_SUBTRACT", offset);
    }
    case OpCode::Multiply: {
      return SimpleInstruction("OP_MULTIPLY", offset);
    }
    case OpCode::Divide: {
      return SimpleInstruction("OP_DIVIDE", offset);
    }
    case OpCode::Return: {
      return SimpleInstruction("OP_RETURN", offset);
    }
    default: {
      printf("Unknown opcode %d\n", byte);
      return offset + 1;
    }
  }
}

auto Chunk::Disassemble() const -> const void {
  printf("==========\n");
  for (u32 offset = 0; offset < this->count;) {
    offset = this->PrintAtOffset(offset);
  }
}
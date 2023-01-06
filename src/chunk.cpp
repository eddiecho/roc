#include "chunk.h"

#include <stdio.h>

#include "common.h"
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

func Chunk::AddLine(u32 line) -> void {
  if (this->lines.count == 0) {
    this->lines.Append({this->count, line});
  } else {
    Range<u32> prev = this->lines[this->lines.count - 1];
    if (prev.val != line) {
      this->lines.Append({this->count, line});
    }
  }
}

func Chunk::AddChunk(u8 byte, u32 line) -> void {
  this->AddLine(line);
  this->Append(byte);
}

#define SMALL_CONST_POOL_SIZE 256
func Chunk::AddConstant(Value val, u32 line) -> void {
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

func Chunk::SimpleInstruction(const char* name, int offset) const -> int {
  printf("%s\n", name);
  return offset + 1;
}

func Chunk::ConstantInstruction(int offset) const -> int {
  u8 const_idx = (*this)[offset + 1];
  printf("%-16s %04d %04d %04d ' ", "OP_CONSTANT", 0, 0, const_idx);
  this->constants[const_idx].Print();
  printf("\n");

  return offset + 2;
}

func Chunk::ConstantLongInstruction(int offset) const -> int {
  u8 one = (*this)[offset + 1];
  u8 two = (*this)[offset + 2];
  u8 thr = (*this)[offset + 3];
  printf("%-16s %04d %04d %04d ' ", "OP_CONSTANT_LONG", one, two, thr);

  u32 idx = 0;
  idx |= (one << 16);
  idx |= (two << 8);
  idx |= (thr);

  this->constants[idx].Print();
  printf("\n");

  return offset + 4;
}

func Chunk::PrintAtOffset(int offset) const -> const int {
  printf("%04d ", offset);

  u32 line_idx = this->lines.Search(offset);
  u32 line = this->lines[line_idx].val;
  printf("%4d ", line);

  u8 byte = (*this)[offset];
  auto instruction = static_cast<OpCode>(byte);

  switch (instruction) {
    case OpCode::Constant: {
      return this->ConstantInstruction(offset);
    }
    case OpCode::ConstantLong: {
      return this->ConstantLongInstruction(offset);
    }
    case OpCode::Negate: {
      return this->SimpleInstruction("OP_NEGATE", offset);
    }
    case OpCode::Add: {
      return this->SimpleInstruction("OP_ADD", offset);
    }
    case OpCode::Subtract: {
      return this->SimpleInstruction("OP_SUBTRACT", offset);
    }
    case OpCode::Multiply: {
      return this->SimpleInstruction("OP_MULTIPLY", offset);
    }
    case OpCode::Divide: {
      return this->SimpleInstruction("OP_DIVIDE", offset);
    }
    case OpCode::True: {
      return this->SimpleInstruction("LIT_TRUE", offset);
    }
    case OpCode::False: {
      return this->SimpleInstruction("LIT_FALSE", offset);
    }
    case OpCode::Return: {
      return this->SimpleInstruction("OP_RETURN", offset);
    }
    case OpCode::Not: {
      return this->SimpleInstruction("OP_NOT", offset);
    }
    case OpCode::Equality: {
      return this->SimpleInstruction("OP_EQUAL", offset);
    }
    case OpCode::Greater: {
      return this->SimpleInstruction("OP_GREATER", offset);
    }
    case OpCode::Less: {
      return this->SimpleInstruction("OP_LESS", offset);
    }
    default: {
      printf("Unknown opcode %d\n", byte);
      return offset + 1;
    }
  }
}

func Chunk::Disassemble() const -> const void {
  printf("==========\n");
  for (u32 offset = 0; offset < this->count;) {
    offset = this->PrintAtOffset(offset);
  }
}


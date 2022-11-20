#pragma once

#include <stdio.h>
#include <tuple>

#include "chunk.h"
#include "value.h"

Chunk::Chunk() noexcept {
  this->init();
  this->constants.init();
  this->lines.init();
}

Chunk::~Chunk() {
  this->lines.deinit();
  this->constants.deinit();
  this->deinit();
}

auto Chunk::addLine(u32 line) -> void {
  if (this->lines.count == 0) {
    this->lines.append({this->count, line});
  } else {
    Range<u32> prev = this->lines[this->lines.count - 1];
    if (prev.val != line) {
      this->lines.append({this->count, line});
    }
  }
}

auto Chunk::addChunk(u8 byte, u32 line) -> void {
  this->addLine(line);
  this->append(byte);
}

#define SMALL_CONST_POOL_SIZE 1
auto Chunk::addConstant(Value val, u32 line) -> void {
  this->addLine(line);

  if (this->constants.count < SMALL_CONST_POOL_SIZE) {
    this->append(OpCode::CONSTANT);
    this->append((u8)this->constants.count);
  } else {
    this->append(OpCode::CONSTANT_LONG);

    u32 count = this->constants.count;
    this->append(count >> 16);
    this->append(count >> 8);
    this->append(count);
  }

  this->constants.append(val);
}

auto static simpleInstruction(const char* name, int offset) -> int {
  printf("%s\n", name);
  return offset + 1;
}

auto static constantInstruction(Chunk* chunk, int offset) -> int {
  u8 const_idx = chunk->data[offset + 1];
  printf("%-16s %04d %04d %04d ' ", "OP_CONSTANT", 0, 0, const_idx);
  printValue(chunk->constants.data[const_idx]);
  printf("\n");

  return offset + 2;
}

auto static constantLongInstruction(Chunk* chunk, int offset) -> int {
  u8 one = chunk->data[offset + 1];
  u8 two = chunk->data[offset + 2];
  u8 thr = chunk->data[offset + 3];
  printf("%-16s %04d %04d %04d ' ", "OP_CONSTANT_LONG", one, two, thr);

  u32 idx = 0;
  idx |= (one << 16);
  idx |= (two << 8);
  idx |= (thr);

  printValue(chunk->constants.data[idx]);
  printf("\n");

  return offset + 4;
}

auto Chunk::printAtOffset(int offset) -> int {
  printf("%04d ", offset);

  u32 line_idx = this->lines.search(offset);
  u32 line = this->lines[line_idx].val;
  printf("%4d ", line);

  u8 instruction = this->data[offset];
  switch (instruction) {
    case OpCode::CONSTANT: {
      return constantInstruction(this, offset);
    }
    case OpCode::CONSTANT_LONG: {
      return constantLongInstruction(this, offset);
    }
    case OpCode::RETURN: {
      return simpleInstruction("OP_RETURN", offset);
    }
    default: {
      printf("Unknown opcode %d\n", instruction);
      return offset + 1;
    }
  }
}

auto Chunk::disassemble() -> void {
  printf("==========\n");
  for (u32 offset = 0; offset < this->count;) {
    offset = this->printAtOffset(offset);
  }
}

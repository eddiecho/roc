#include "chunk.h"

#include <stdio.h>

#include "common.h"
#include "value.h"

Chunk::Chunk() noexcept {
  this->Init();
  this->constants.Init();
  this->lines.Init();
}

fnc Chunk::AddLine(u64 line) -> void {
  if (this->lines.count == 0) {
    this->lines.Append({this->count, line});
  } else {
    Range<u64> prev = this->lines[this->lines.count - 1];
    if (prev.val != line) {
      this->lines.Append({this->count, line});
    }
  }
}

fnc Chunk::AddChunk(u8 byte, u64 line) -> u64 {
  this->AddLine(line);
  return this->Append(byte);
}

fnc Chunk::AddChunk(u8* bytes, u64 count, u64 line) -> u64 {
  this->AddLine(line);
  return this->Append(bytes, count);
}

#define SMALL_CONST_POOL_SIZE 256
fnc Chunk::AddConstant(Value val, u64 line) -> u64 {
  this->AddLine(line);

  if (this->constants.count < SMALL_CONST_POOL_SIZE) {
    this->Append(static_cast<u8>(OpCode::Constant));
    this->Append((u8)this->constants.count);
  } else {
    this->Append(static_cast<u8>(OpCode::ConstantLong));

    u64 count = this->constants.count;
    this->Append(count >> 24);
    this->Append(count >> 16);
    this->Append(count >> 8);
    this->Append(count);
  }

  return this->constants.Append(val);
}

fnc Chunk::SimpleInstruction(const char* name, int offset) const -> int {
  printf("%s\n", name);
  return offset + 1;
}

fnc Chunk::ConstantInstruction(int offset) const -> int {
  u8 const_idx = (*this)[offset + 1];
  printf("%-16s %04d %04d %04d ' ", "OP_CONSTANT", 0, 0, const_idx);
  this->constants[const_idx].Print();
  printf("\n");

  return offset + 2;
}

fnc Chunk::ConstantLongInstruction(int offset) const -> int {
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

fnc Chunk::ByteInstruction(const char* name, int offset) const -> int {
  u8 idx = this->data[offset + 1];
  printf("%-16s %4d\n", name, idx);
  return offset + 2;
}

fnc Chunk::JumpInstruction(const char* name, int sign, int offset) const -> int {
  u32 jmp = (u32)(this->data[offset + 1] << 24);
  // jmp |= (u32)(this->data[offset + 1] << 24);
  printf("%-16s %4d -> %d\n", name, offset, offset + 5 + sign + jmp);
  return offset + 5;
}

fnc Chunk::GlobalInstruction(const char* name, int offset) const -> int {
  u8 one = (*this)[offset + 1];
  u8 two = (*this)[offset + 2];
  u8 thr = (*this)[offset + 3];
  u8 fou = (*this)[offset + 4];

  printf("%-16s %04d %04d %04d %04d ' \n", name, one, two, thr, fou);

  return offset + 5;
}

fnc Chunk::PrintAtOffset(int offset) const -> const int {
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
    case OpCode::String: {
      return this->GlobalInstruction("OP_STRING", offset);
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
    case OpCode::Pop: {
      return this->SimpleInstruction("OP_POP", offset);
    }
    case OpCode::SetGlobal: {
      return this->GlobalInstruction("OP_SETGLOBAL", offset);
    }
    case OpCode::GetGlobal: {
      return this->GlobalInstruction("OP_GETGLOBAL", offset);
    }
    case OpCode::SetLocal: {
      return this->GlobalInstruction("OP_SETLOCAL", offset);
    }
    case OpCode::GetLocal: {
      return this->GlobalInstruction("OP_GETLOCAL", offset);
    }
    case OpCode::Jump: {
      return this->JumpInstruction("OP_JUMP", 1, offset);
    }
    case OpCode::JumpFalse: {
      return this->JumpInstruction("OP_JFALSE", 1, offset);
    }
    case OpCode::JumpTrue: {
      return this->JumpInstruction("OP_JTRUE", 1, offset);
    }
    case OpCode::Loop: {
      return this->JumpInstruction("OP_LOOP", -1, offset);
    }
    case OpCode::Invoke: {
      return this->ByteInstruction("OP_INVOKE", offset);
    }
    case OpCode::Closure: {
      offset++;
      u8 closure_idx = this->data[offset++];
      printf("%-16s %4d ", "OP_CLOSURE", closure_idx);
      this->constants[closure_idx].Print();
      printf("\n");

      return offset;
    }
    default: {
      printf("Unknown opcode %d\n", byte);
      return offset + 1;
    }
  }
}

fnc Chunk::Disassemble() const -> const void {
  printf("==========\n");
  for (u32 offset = 0; offset < this->count;) {
    offset = this->PrintAtOffset(offset);
  }
}


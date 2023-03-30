#include "chunk.h"

#include <stdio.h>

#include "common.h"
#include "memory.h"
#include "value.h"

fnc Chunk::Init() -> void {
  this->bytecode.Init();
  this->locals.Init();
  this->lines.Init();
}

fnc Chunk::Deinit() -> void {
  this->bytecode.Deinit();
  this->locals.Deinit();
  this->lines.Deinit();
}

fnc Chunk::Count() const -> u64 {
  return this->bytecode.count;
}

fnc Chunk::BaseInstructionPointer() const -> u8* {
  return this->bytecode.data;
}

fnc Chunk::AddLine(u64 line) -> void {
  auto count = this->bytecode.count;

  if (this->lines.count == 0) {
    this->lines.Append({count, line});
  } else {
    auto prev = this->lines[this->lines.count - 1];
    if (prev.val != line) {
      this->lines.Append({count, line});
    }
  }
}

fnc Chunk::AddInstruction(u8 byte, u64 line) -> u64 {
  this->AddLine(line);
  return this->bytecode.Append(byte);
}

fnc Chunk::AddInstruction(u8* bytes, u64 count, u64 line) -> u64 {
  this->AddLine(line);
  return this->bytecode.Append(bytes, count);
}

#define SMALL_CONST_POOL_SIZE 256
fnc Chunk::AddLocal(Value val, u64 line) -> u64 {
  this->AddLine(line);

  u32 idx = this->locals.count;

  if (idx < SMALL_CONST_POOL_SIZE) {
    this->bytecode.Append(static_cast<u8>(OpCode::Constant));
    this->bytecode.Append((u8)idx);
  } else {
    this->bytecode.Append(static_cast<u8>(OpCode::ConstantLong));

    this->bytecode.Append(idx >> 24);
    this->bytecode.Append(idx >> 16);
    this->bytecode.Append(idx >> 8);
    this->bytecode.Append(idx);
  }

  return this->locals.Append(val);
}

fnc Chunk::SimpleInstruction(const char* name, int offset) const -> int {
  printf("%s\n", name);
  return offset + 1;
}

fnc Chunk::ConstantInstruction(int offset) const -> int {
  u8 const_idx = this->bytecode[offset + 1];
  printf("%-16s %04d %04d %04d ' ", "OP_CONSTANT", 0, 0, const_idx);
  this->locals[const_idx].Print();
  printf("\n");

  return offset + 2;
}

fnc Chunk::ConstantLongInstruction(int offset) const -> int {
  u8 one = this->bytecode[offset + 1];
  u8 two = this->bytecode[offset + 2];
  u8 thr = this->bytecode[offset + 3];
  printf("%-16s %04d %04d %04d ' ", "OP_CONSTANT_LONG", one, two, thr);

  u32 idx = 0;
  idx |= (one << 16);
  idx |= (two << 8);
  idx |= (thr);

  this->locals[idx].Print();
  printf("\n");

  return offset + 4;
}

fnc Chunk::ByteInstruction(const char* name, int offset) const -> int {
  u8 idx = this->bytecode[offset + 1];
  printf("%-16s %4d\n", name, idx);
  return offset + 2;
}

fnc Chunk::JumpInstruction(const char* name, int sign, int offset) const -> int {
  auto location = this->bytecode.data + 1;
  auto as_int = reinterpret_cast<u32*>(location);
  u32 jmp = *location;

  printf("%-16s %4d -> %d\n", name, offset, offset + 5 + sign + jmp);
  return offset + 5;
}

fnc Chunk::GlobalInstruction(const char* name, int offset) const -> int {
  u8 one = this->bytecode[offset + 1];
  u8 two = this->bytecode[offset + 2];
  u8 thr = this->bytecode[offset + 3];
  u8 fou = this->bytecode[offset + 4];

  printf("%-16s %04d %04d %04d %04d ' \n", name, one, two, thr, fou);

  return offset + 5;
}

// @TODO(eddie) - just make every single opcode 64bits
// @FIXME(eddie) - another pass needed
fnc Chunk::PrintAtOffset(int offset) const -> const int {
  printf("%04d ", offset);

  u32 line_idx = this->lines.Search(offset);
  u32 line = this->lines[line_idx].val;
  printf("%4d ", line);

  u8 byte = this->bytecode[offset];
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
    case OpCode::ReturnVoid: {
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
    case OpCode::SetUpvalue: {
      return this->GlobalInstruction("OP_SETUPVALUE", offset);
    }
    case OpCode::GetUpvalue: {
      return this->GlobalInstruction("OP_GETUPVALUE", offset);
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
      return this->ByteInstruction("OP_INVOKE", offset) + 3;
    }
    case OpCode::Closure: {
      u8 closure_idx = this->bytecode[offset + 1 ];
      printf("%-16s %4d ", "OP_CLOSURE", closure_idx);
      this->locals[closure_idx].Print();
      printf("\n");

      return offset + 4;
    }
    default: {
      printf("Unknown opcode %d\n", byte);
      return offset + 1;
    }
  }
}

fnc Chunk::Disassemble() const -> const void {
  printf("==========\n");
  for (u32 offset = 0; offset < this->bytecode.count;) {
    offset = this->PrintAtOffset(offset);
  }
}

fnc ChunkManager::Alloc() -> Chunk* {
  if (this->capacity < this->count + 1) {
    auto old_cap = this->capacity;
    this->capacity = GROW_CAPACITY(old_cap);
    this->chunks = GROW_ARRAY(Chunk, this->chunks, old_cap, this->capacity);
  }

  Chunk* chunk = &this->chunks[this->count++];
  chunk->Init();

  return chunk;
}

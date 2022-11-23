#pragma once

#include "common.h"
#include "vm.h"

#define DEBUG_TRACE_EXECUTION

auto VirtualMachine::init() -> void {}

auto VirtualMachine::deinit() -> void {}

auto VirtualMachine::interpret(Chunk *chunk) -> InterpretError {
  this->chunk = chunk;
  this->instructionPointer = chunk->data;

#define READ_BYTE() (*this->instructionPointer++)
#define READ_CONSTANT() (this->chunk->constants.data[READ_BYTE()])

  while (1) {
    u8 instruction;

#ifdef DEBUG_TRACE_EXECUTION
    this->chunk->printAtOffset((int) (this->instructionPointer - this->chunk->data));
#endif

    switch(instruction = READ_BYTE()) {
      case OpCode::Return: {
        return InterpretError::Success;
      }
      case OpCode::Constant: {
        Value constant = READ_CONSTANT();
        printValue(constant);
        printf("\n");
        break;
      }
      case OpCode::ConstantLong: {
        u32 idx = 0;
        idx |= ((u8) READ_BYTE() << 16);
        idx |= ((u8) READ_BYTE() << 8);
        idx |= (u8) READ_BYTE();

        Value constant = this->chunk->constants.data[idx];
        printValue(constant);
        printf("\n");

        break;
      }
      default: {
        break;
      }
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT

  return InterpretError::Success;
}

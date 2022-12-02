#pragma once

#include "common.h"
#include "vm.h"

#define DEBUG_TRACE_EXECUTION

auto VirtualMachine::Init() -> void {
  // reset stack pointer
  this->stackTop = this->stack;
}

auto VirtualMachine::Deinit() -> void {}

auto VirtualMachine::Push(Value value) -> void {
  // TODO - this is bad
  if (this->stackTop - this->stack > VM_STACK_MAX) {
    exit(1);
  }

  *this->stackTop = value;
  this->stackTop++;
}

auto VirtualMachine::Pop() -> Value {
  this->stackTop--;
  return *this->stackTop;
}

auto VirtualMachine::Interpret(const char* src) -> InterpretError {
  return InterpretError::Success;
}

auto VirtualMachine::Interpret(Chunk *chunk) -> InterpretError {
  this->chunk = chunk;
  this->instructionPointer = chunk->data_;

#define READ_BYTE() (*this->instructionPointer++)
#define READ_CONSTANT() (this->chunk->constants_.data_[READ_BYTE()])
// TODO - this isnt good for operator overloading
#define BINARY_OP(op) \
  do { \
    double b = this->Pop(); \
    double a = this->Pop(); \
    this->Push(a op b); \
  } while(0)

  u8 byte;
  while (1) {

#ifdef DEBUG_TRACE_EXECUTION
    this->chunk->PrintAtOffset((int) (this->instructionPointer - this->chunk->data_));
#endif

    byte = READ_BYTE();
    OpCode instruction = static_cast<OpCode>(byte);

    switch(instruction) {
      case OpCode::Return: {
        PrintValue(this->Pop());
        printf("\n");

        return InterpretError::Success;
      }
      case OpCode::Constant: {
        Value constant = READ_CONSTANT();
        this->Push(constant);
        break;
      }
      case OpCode::ConstantLong: {
        u32 idx = 0;
        idx |= ((u8) READ_BYTE() << 16);
        idx |= ((u8) READ_BYTE() << 8);
        idx |= (u8) READ_BYTE();

        Value constant = this->chunk->constants_.data_[idx];
        this->Push(constant);
        break;
      }
      case OpCode::Add:      { BINARY_OP(+); break; }
      case OpCode::Subtract: { BINARY_OP(-); break; }
      case OpCode::Multiply: { BINARY_OP(*); break; }
      case OpCode::Divide:   { BINARY_OP(/); break; }
      case OpCode::Negate: {
        this->Push(-this->Pop());
        break;
      }
      default: {
        break;
      }
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP

  return InterpretError::Success;
}

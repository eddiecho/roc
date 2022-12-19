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
  // TODO(eddie) - this is bad error handling
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

auto VirtualMachine::Interpret(Chunk *chunk) -> InterpretError {
  this->chunk = chunk;
  this->instructionPointer = chunk->data;

#define READ_BYTE() (*this->instructionPointer++)
#define READ_CONSTANT() (this->chunk->constants[READ_BYTE()])
// TODO(eddie) - this isnt good for operator overloading
#define BINARY_OP(op) \
  do { \
    double b = this->Pop().as.number; \
    double a = this->Pop().as.number; \
    this->Push(a op b); \
  } while (0)

  u8 byte;
  while (1) {
#ifdef DEBUG_TRACE_EXECUTION
    this->chunk->PrintAtOffset(static_cast<int>(this->instructionPointer - this->chunk->data));
#endif

    byte = READ_BYTE();
    OpCode instruction = static_cast<OpCode>(byte);

    switch (instruction) {
      case OpCode::Return: {
        this->Pop().Print();
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

        Value constant = this->chunk->constants[idx];
        this->Push(constant);
        break;
      }
      case OpCode::Add:      { BINARY_OP(+); break; }
      case OpCode::Subtract: { BINARY_OP(-); break; }
      case OpCode::Multiply: { BINARY_OP(*); break; }
      case OpCode::Divide:   { BINARY_OP(/); break; }
      case OpCode::Negate: {
        this->Push(-this->Pop().as.number);
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

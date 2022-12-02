#pragma once

#include "common.h"
#include "vm.h"

#define DEBUG_TRACE_EXECUTION

auto VirtualMachine::init() -> void {
  // reset stack pointer
  this->stackTop = this->stack;
}

auto VirtualMachine::deinit() -> void {}

auto VirtualMachine::push(Value value) -> void {
  // TODO - this is bad
  if (this->stackTop - this->stack > VM_STACK_MAX) {
    exit(1);
  }

  *this->stackTop = value;
  this->stackTop++;
}

auto VirtualMachine::pop() -> Value {
  this->stackTop--;
  return *this->stackTop;
}

auto VirtualMachine::interpret(const char* src) -> InterpretError {
  return InterpretError::Success;
}

auto VirtualMachine::interpret(Chunk *chunk) -> InterpretError {
  this->chunk = chunk;
  this->instructionPointer = chunk->data;

#define READ_BYTE() (*this->instructionPointer++)
#define READ_CONSTANT() (this->chunk->constants.data[READ_BYTE()])
// TODO - this isnt good for operator overloading
#define BINARY_OP(op) \
  do { \
    double b = this->pop(); \
    double a = this->pop(); \
    push(a op b); \
  } while(0)

  while (1) {
    u8 instruction;

#ifdef DEBUG_TRACE_EXECUTION
    this->chunk->printAtOffset((int) (this->instructionPointer - this->chunk->data));
#endif

    switch(instruction = READ_BYTE()) {
      case OpCode::Return: {
        printValue(this->pop());
        printf("\n");

        return InterpretError::Success;
      }
      case OpCode::Constant: {
        Value constant = READ_CONSTANT();
        this->push(constant);
        break;
      }
      case OpCode::ConstantLong: {
        u32 idx = 0;
        idx |= ((u8) READ_BYTE() << 16);
        idx |= ((u8) READ_BYTE() << 8);
        idx |= (u8) READ_BYTE();

        Value constant = this->chunk->constants.data[idx];
        this->push(constant);
        break;
      }
      case OpCode::Add:      { BINARY_OP(+); break; }
      case OpCode::Subtract: { BINARY_OP(-); break; }
      case OpCode::Multiply: { BINARY_OP(*); break; }
      case OpCode::Divide:   { BINARY_OP(/); break; }
      case OpCode::Negate: {
        this->push(-this->pop());
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

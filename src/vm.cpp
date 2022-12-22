#pragma once

#include "vm.h"

#include "common.h"

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

auto VirtualMachine::Peek(int dist) const -> Value {
  return this->stackTop[-1 - dist];
}

auto VirtualMachine::Reset() -> void {
  this->stackTop = this->stack;
}

auto VirtualMachine::RuntimeError(const char* msg, ...) -> void {
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fputs("\n", stderr);

  size_t inst = this->instructionPointer - this->chunk->data - 1;
  int line = this->chunk->lines[inst].val;

  fprintf(stderr, "[line %d] in file\n", line);
  this->Reset();
}

auto VirtualMachine::Interpret(Chunk *chunk) -> InterpretError {
  this->chunk = chunk;
  this->instructionPointer = chunk->data;

#define READ_BYTE() (*this->instructionPointer++)
#define READ_CONSTANT() (this->chunk->constants[READ_BYTE()])

  u8 byte;
  while (1) {
#ifdef DEBUG_TRACE_EXECUTION
    this->chunk->PrintAtOffset(
      static_cast<int>(this->instructionPointer - this->chunk->data));
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
        idx |= ((u8) READ_BYTE());

        Value constant = this->chunk->constants[idx];
        this->Push(constant);
        break;
      }
      case OpCode::Add:      {
        Value b = this->Pop();
        Value a = this->Pop();

        this->Push(Value(a.as.number + b.as.number));
        break;
      }
      case OpCode::Subtract: {
        Value b = this->Pop();
        Value a = this->Pop();

        this->Push(Value(a.as.number - b.as.number));
        break;
      }
      case OpCode::Multiply: {
        Value b = this->Pop();
        Value a = this->Pop();

        this->Push(Value(a.as.number * b.as.number));
        break;
      }
      case OpCode::Divide:   {
        Value b = this->Pop();
        Value a = this->Pop();

        this->Push(Value(a.as.number / b.as.number));
        break;
      }
      case OpCode::Negate: {
        this->Push(-this->Pop().as.number);
        break;
      }
      case OpCode::False: {
        this->Push(false);
        break;
      }
      case OpCode::True: {
        this->Push(true);
        break;
      }
      case OpCode::Not: {
        Value val = this->Pop();
        this->Push(!val.as.boolean);
        break;
      }
      case OpCode::Equality: {
        Value b = this->Pop();
        Value a = this->Pop();
        this->Push(a == b);
        break;
      }
      case OpCode::Greater: {
        Value b = this->Pop();
        Value a = this->Pop();
        this->Push(Value(a.as.number > b.as.number));
        break;
      }
      case OpCode::Less: {
        Value b = this->Pop();
        Value a = this->Pop();
        this->Push(Value(a.as.number < b.as.number));
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

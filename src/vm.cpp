#include "vm.h"

#include <stdarg.h>
#include <stdlib.h>

#include "arena.h"
#include "common.h"

auto VirtualMachine::Init() -> void {
  // reset stack pointer
  this->stack_top = this->stack;
}

auto VirtualMachine::Deinit() -> void {}

auto VirtualMachine::Push(Value value) -> void {
  // TODO(eddie) - this is bad error handling
  if (this->stack_top - this->stack > VM_STACK_MAX) {
    exit(1);
  }

  *this->stack_top = value;
  this->stack_top++;
}

auto VirtualMachine::Pop() -> Value {
  this->stack_top--;
  return *this->stack_top;
}

auto VirtualMachine::Peek() const -> Value {
  return *this->stack_top;
}

auto VirtualMachine::Peek(int dist) const -> Value {
  return this->stack_top[-1 - dist];
}

auto VirtualMachine::Reset() -> void { this->stack_top = this->stack; }

auto VirtualMachine::RuntimeError(const char* msg, ...) -> void {
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fputs("\n", stderr);

  size_t inst = this->inst_ptr - this->chunk->data - 1;
  int line = this->chunk->lines[inst].val;

  fprintf(stderr, "[line %d] in file\n", line);
  this->Reset();
}

auto VirtualMachine::Interpret(Chunk* chunk) -> InterpretError {
  this->chunk = chunk;
  this->inst_ptr = chunk->data;

#define READ_BYTE() (*this->inst_ptr++)
#define READ_CONSTANT() (this->chunk->constants[READ_BYTE()])

  u8 byte;
  while (1) {
#ifdef DEBUG_TRACE_EXECUTION
    this->chunk->PrintAtOffset(
        static_cast<int>(this->inst_ptr - this->chunk->data));
#endif

    byte = READ_BYTE();
    auto instruction = static_cast<OpCode>(byte);

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
        idx |= ((u8)READ_BYTE() << 16);
        idx |= ((u8)READ_BYTE() << 8);
        idx |= ((u8)READ_BYTE());

        Value constant = this->chunk->constants[idx];
        this->Push(constant);
        break;
      }
      case OpCode::Add: {
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
      case OpCode::Divide: {
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

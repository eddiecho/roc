#include "vm.h"

#include <stdarg.h>
#include <stdlib.h>

#include "absl/container/flat_hash_set.h"

#include "common.h"
#include "dynamic_array.h"
#include "object.h"

fnc VirtualMachine::Init() -> void {
  // reset stack pointer
  this->stack_top = this->stack;
}

fnc VirtualMachine::Deinit() -> void {
  this->stack_top = this->stack;
  this->string_pool->Deinit();
  this->chunk->Deinit();
  this->object_pool->Clear();
}

fnc VirtualMachine::Push(Value value) -> void {
  // TODO(eddie) - this is bad error handling
  if (this->stack_top - this->stack > VM_STACK_MAX) {
    exit(1);
  }

  *this->stack_top = value;
  this->stack_top++;
}

fnc VirtualMachine::Pop() -> Value {
  this->stack_top--;
  return *this->stack_top;
}

fnc VirtualMachine::Peek() const -> Value { return *this->stack_top; }

fnc VirtualMachine::Peek(int dist) const -> Value {
  return this->stack_top[-1 - dist];
}

fnc VirtualMachine::RuntimeError(const char* msg, ...) -> void {
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fputs("\n", stderr);

  size_t inst = this->inst_ptr - this->chunk->data - 1;
  int line = this->chunk->lines[inst].val;

  fprintf(stderr, "[line %d] in file\n", line);
  // reset stack pointer
  this->stack_top = this->stack;
}

fnc VirtualMachine::Interpret(
  Chunk* chunk,
  StringPool* string_pool,
  Arena<Object>* object_pool
) -> InterpretError {
  this->chunk = chunk;
  this->inst_ptr = chunk->data;
  this->string_pool = string_pool;
  this->object_pool = object_pool;

#define READ_BYTE() (*this->inst_ptr++)
#define READ_INT() *(u32*)(this->inst_ptr); this->inst_ptr += sizeof(u32)
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
        Value val = this->Pop();
        val.Print();
        printf("\n");

        return InterpretError::Success;
      }
      case OpCode::Constant: {
        Value constant = READ_CONSTANT();
        this->Push(constant);
        break;
      }
      case OpCode::ConstantLong: {
        u32 idx = READ_INT();

        Value constant = this->chunk->constants[idx];
        this->Push(constant);
        break;
      }
      case OpCode::String: {
        u32 idx = READ_INT();

        auto str = this->string_pool->Nth(idx);
        Value constant = Value(str);
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
      case OpCode::Pop: {
        this->Pop();
        break;
      }
      case OpCode::SetGlobal: {
        u32 idx = READ_INT();
        Value val = this->Pop();
        Object* slot = this->object_pool->Nth(idx);
        slot = std::move(val.as.object);
        break;
      }
      case OpCode::GetGlobal: {
        u32 idx = READ_INT();
        this->Push(Value(this->object_pool->Nth(idx)));
        break;
      }
      case OpCode::SetLocal: {
        u32 idx = READ_INT();
        this->stack[idx] = this->Peek();
        break;
      }
      case OpCode::GetLocal: {
        u32 idx = READ_INT();
        this->Push(this->stack[idx]);
        break;
      }
      case OpCode::Jump: {
        u32 offset = READ_INT();
        this->inst_ptr += offset;
        break;
      }
      case OpCode::JumpFalse: {
        u32 offset = READ_INT();
        Value condition = this->Peek();
        if (!condition.IsTruthy()) {
          this->inst_ptr += offset;
        }
        break;
      }
      case OpCode::JumpTrue: {
        u32 offset = READ_INT();
        Value condition = this->Peek();
        if (condition.IsTruthy()) {
          this->inst_ptr += offset;
        }
        break;
      }
      case OpCode::Loop: {
        u32 offset = READ_INT();
        this->inst_ptr -= offset;
        break;
      }
      default: {
        printf("Unimplemented OpCode reached???");
        break;
      }
    }
  }

#undef READ_BYTE
#undef READ_INT
#undef READ_CONSTANT

  return InterpretError::Success;
}

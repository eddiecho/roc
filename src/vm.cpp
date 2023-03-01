#include "vm.h"

#include <stdarg.h>
#include <stdlib.h>

#include "absl/container/flat_hash_set.h"

#include "common.h"
#include "dynamic_array.h"
#include "object.h"
#include "value.h"

fnc VirtualMachine::Init() -> void {
  // reset stack pointer
  this->stack_top = this->stack;
}

fnc VirtualMachine::Deinit() -> void {
  this->stack_top = this->stack;
  this->frame_count = 0;

  if (this->string_pool != nullptr) {
    this->string_pool->Deinit();
  }

  if (this->object_pool != nullptr) {
    this->object_pool->Clear();
  }
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

fnc VirtualMachine::RuntimeError(const char* msg, ...) -> InterpretError {
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fputs("\n", stderr);

  for (int i = this->frame_count - 1; i >= 0; i--) {
    StackFrame* frame = &this->frames[i];
    auto func = frame->function;
    u64 inst = frame->inst_ptr - func->as.function.chunk.data - 1;

    auto line_range = func->as.function.chunk.lines[inst];
    fprintf(stderr, "[line %lu] in ", line_range.val);
    fprintf(stderr, "%s\n", func->name);
  }

  // reset stack pointer
  this->stack_top = this->stack;

  return InterpretError::RuntimeError;
}

fnc VirtualMachine::Interpret(
  Object::Function* func,
  StringPool* string_pool,
  Arena<Object>* object_pool
) -> InterpretError {

  StackFrame* frame;
  frame = &this->frames[this->frame_count - 1];
  frame->function = func;
  frame->inst_ptr = func->as.function.chunk.data;
  frame->locals = this->stack;

  this->string_pool = string_pool;
  this->object_pool = object_pool;

  // func->as.function.chunk.Disassemble();

#define CurrentFrame() (&this->frames[this->frame_count - 1])
#define GET_CHUNK() (CurrentFrame()->function->as.function.chunk)
#define READ_BYTE() (*CurrentFrame()->inst_ptr++)
#define READ_INT() *(u32*)(CurrentFrame()->inst_ptr); (CurrentFrame())->inst_ptr += sizeof(u32)
#define READ_CONSTANT() (GET_CHUNK().constants[READ_BYTE()])

  u8 byte;
  while (1) {

#ifdef DEBUG_TRACE_EXECUTION
    func->as.function.chunk.PrintAtOffset(
        static_cast<int>(CurrentFrame()inst_ptr - GET_CHUNK().data));
#endif

    byte = READ_BYTE();
    auto instruction = static_cast<OpCode>(byte);

    switch (instruction) {
      case OpCode::Return: {
        Value val = this->Pop();
        this->frame_count--;
        if (this->frame_count == 0) {
          // the book has this here because it uses the first stack slot
          // implicitly as the top level function
          // this->Pop();
          return InterpretError::Success;
        }

        this->stack_top = CurrentFrame()->locals;
        this->Push(val);
        frame = &this->frames[this->frame_count - 1];

        break;
      }
      case OpCode::Constant: {
        Value constant = READ_CONSTANT();
        this->Push(constant);
        break;
      }
      case OpCode::ConstantLong: {
        u32 idx = READ_INT();

        Value constant = GET_CHUNK().constants[idx];
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
        CurrentFrame()->locals[idx] = this->Peek();
        break;
      }
      case OpCode::GetLocal: {
        u32 idx = READ_INT();
        this->Push(CurrentFrame()->locals[idx]);
        break;
      }
      case OpCode::Jump: {
        u32 offset = READ_INT();
        CurrentFrame()->inst_ptr += offset;
        break;
      }
      case OpCode::JumpFalse: {
        u32 offset = READ_INT();
        Value condition = this->Peek();
        if (!condition.IsTruthy()) {
          CurrentFrame()->inst_ptr += offset;
        }
        break;
      }
      case OpCode::JumpTrue: {
        u32 offset = READ_INT();
        Value condition = this->Peek();
        if (condition.IsTruthy()) {
          CurrentFrame()->inst_ptr += offset;
        }
        break;
      }
      case OpCode::Loop: {
        u32 offset = READ_INT();
        CurrentFrame()->inst_ptr -= offset;
        break;
      }
      case OpCode::Invoke: {
        u32 argc = READ_INT();
        auto function_base = this->Peek(argc);
        if (function_base.IsObject()) {
          StackFrame* new_frame = &this->frames[this->frame_count++];
          auto new_func = static_cast<Object::Function*>(function_base.as.object);

          if (argc != new_func->as.function.arity) {
            return this->RuntimeError("Expected %d arguments to function but got %d",
                                      new_func->as.function.arity, argc);
          }

          if (this->frame_count == VM_STACK_MAX) {
            return this->RuntimeError("Stack overflow");
          }

          new_frame->function = new_func;
          new_frame->inst_ptr = new_func->as.function.chunk.data;
          new_frame->locals = this->stack_top - argc - 1;

        } else {
          return this->RuntimeError("Can not invoke non function object");
        }
        break;
      }
      default: {
        printf("Unimplemented OpCode %d reached???\n", static_cast<u8>(instruction));
        break;
      }
    }
  }

#undef READ_BYTE
#undef READ_INT
#undef READ_CONSTANT

  return InterpretError::Success;
}

#undef GET_CHUNK

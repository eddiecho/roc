#include "vm.h"

#include <stdarg.h>
#include <stdlib.h>

#include "absl/container/flat_hash_set.h"
#include <string>

#include "common.h"
#include "dynamic_array.h"
#include "object.h"
#include "utils.h"
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

fnc VirtualMachine::Peek() const -> Value {
  return this->stack_top[-1];
}

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
    auto closure = frame->closure;
    auto func = closure->Unwrap();

    u64 inst = frame->inst_ptr - func->chunk.BaseInstructionPointer() - 1;

    auto line_range = func->chunk.lines[inst];
    auto name = frame->closure == nullptr ? frame->function->name : frame->closure->name;
    fprintf(stderr, "[line %lu] in ", line_range.val);
    fprintf(stderr, "%s\n", name);
  }

  // reset stack pointer
  this->stack_top = this->stack;

  return InterpretError::RuntimeError;
}

fnc VirtualMachine::Invoke(Object::Closure* closure, u32 argc) -> Result<size_t, InterpretError> {
  auto inner_func = *closure->Unwrap();

  if (argc != inner_func.arity) {
    return this->RuntimeError("Expected %d arguments to function but got %d",
                        inner_func.arity, argc);
  }

  if (this->frame_count == VM_STACK_MAX) {
    return this->RuntimeError("Stack overflow");
  }

  StackFrame* ret = &this->frames[this->frame_count++];
  ret->type = FrameType::Closure;
  ret->closure = closure;
  ret->inst_ptr = inner_func.chunk.BaseInstructionPointer();
  // the -1 is so that slot 0 of locals is a reference to Object::Function being called
  ret->locals = this->stack_top - argc - 1;
  ret->chunk = &inner_func.chunk;

  return this->frame_count - 1;
}

fnc VirtualMachine::Invoke(Object::Function* function, u32 argc) -> Result<size_t, InterpretError> {
  if (argc != function->as.function.arity) {
    return this->RuntimeError("Expected %d arguments to function but got %d",
                              function->as.function.arity, argc);
  }

  if (this->frame_count == VM_STACK_MAX) {
    return this->RuntimeError("Stack overflow");
  }

  StackFrame* ret = &this->frames[this->frame_count++];
  ret->type = FrameType::Function;
  ret->function = function;
  ret->inst_ptr = function->as.function.chunk.BaseInstructionPointer();
  // the -1 is so that slot 0 of locals is a reference to Object::Function being called
  ret->locals = this->stack_top - argc - 1;
  ret->chunk = &function->as.function.chunk;

  return this->frame_count - 1;
}

// @TODO(eddie) - big sweep through all the opcodes
// basically everything is actually a 64bit int, but im truncating it down to 32s
fnc VirtualMachine::Interpret(
  Object* obj,
  StringPool* string_pool,
  Arena<Object>* object_pool
) -> InterpretResult {

  Assert(obj != nullptr);
  auto function = static_cast<Object::Function*>(obj);

  // setup initial call stack
  auto frame_result = this->Invoke(function, 0);
  if (frame_result.IsError()) {
    return frame_result.Err();
  }
  auto frame_idx = frame_result.Get();
  auto frame = &this->frames[frame_idx];

  this->string_pool = string_pool;
  this->object_pool = object_pool;

#if 1
  absl::flat_hash_set<std::string_view> function_map;
  frame->chunk->Disassemble();
#endif

#define READ_BYTE() (*frame->inst_ptr++)
#define READ_INT() *(u32*)(frame->inst_ptr); (frame->inst_ptr += sizeof(u32))
#define READ_CONSTANT() (frame->chunk->locals[READ_BYTE()])

  u8 byte;
  while (1) {

    byte = READ_BYTE();
    auto instruction = static_cast<OpCode>(byte);

    switch (instruction) {
      case OpCode::ReturnVoid: {
        this->frame_count--;
        if (this->frame_count == 0) {
          return this->Pop();
        }

        this->stack_top = this->frames[this->frame_count].locals;
        frame = &this->frames[this->frame_count - 1];
        break;
      }
      case OpCode::Return: {
        Value ret_val = this->Pop();
        this->frame_count--;

        this->stack_top = this->frames[this->frame_count].locals;
        this->Push(ret_val);
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

        Value constant = frame->chunk->locals[idx];
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
        Value a = this->Pop();
        this->Push(-a.as.number);
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
        this->Push(Value(a == b));
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
        Value global = this->object_pool->Nth(idx);
        this->Push(global);
        break;
      }
      case OpCode::SetLocal: {
        u32 idx = READ_INT();
        frame->locals[idx] = this->Peek();
        break;
      }
      case OpCode::GetLocal: {
        u32 idx = READ_INT();
        Value local = frame->locals[idx];
        this->Push(local);
        break;
      }
      case OpCode::SetUpvalue: {
        u32 index = READ_INT();
        // lmao thats a lot of indirection
        auto upval = frame->closure->as.closure.upvalues[index];
        *upval->as.upvalue.location = this->Peek();
        break;
      }
      case OpCode::GetUpvalue: {
        u32 index = READ_INT();
        auto upval = frame->closure->as.closure.upvalues[index];
        auto val = upval->as.upvalue.location;
        this->Push(*val);
        break;
      }
      case OpCode::Jump: {
        u32 offset = READ_INT();
        frame->inst_ptr += offset;
        break;
      }
      case OpCode::JumpFalse: {
        u32 offset = READ_INT();
        Value condition = this->Peek();
        if (!condition.IsTruthy()) {
          frame->inst_ptr += offset;
        }
        break;
      }
      case OpCode::JumpTrue: {
        u32 offset = READ_INT();
        Value condition = this->Peek();
        if (condition.IsTruthy()) {
          frame->inst_ptr += offset;
        }
        break;
      }
      case OpCode::Loop: {
        u32 offset = READ_INT();
        frame->inst_ptr -= offset;
        break;
      }
      case OpCode::Invoke: {
        u32 argc = READ_INT();
        auto function_base = this->Peek(argc);
        if (function_base.IsObject()) {

          auto function_obj = function_base.as.object;
          switch (function_obj->type) {
            default:
              return this->RuntimeError("Can not invoke non function object");
            case ObjectType::Function: {
              auto new_function = static_cast<Object::Function*>(function_obj);
              auto new_frame_result = this->Invoke(new_function, argc);
              if (new_frame_result.IsError()) {
                return new_frame_result.Err();
              }

              frame = &this->frames[new_frame_result.Get()];
              break;
            }
            case ObjectType::Closure: {
              auto new_closure = static_cast<Object::Closure*>(function_base.as.object);
              auto new_frame_result = this->Invoke(new_closure, argc);
              if (new_frame_result.IsError()) {
                return new_frame_result.Err();
              }

              frame = &this->frames[new_frame_result.Get()];
              break;
            }
          }

          #if 1
            auto it = function_map.find(function_obj->name);
            if (it == function_map.end()) {
              printf("====== Function: %s\n", function_obj->name);
              frame->chunk->Disassemble();
              function_map.insert(function_obj->name);
            }
          #endif

        } else {
          return this->RuntimeError("Can not invoke non function object");
        }
        break;
      }
      case OpCode::Closure: {
//         u32 idx = READ_INT();

        Object::FunctionData* inner_func;
        switch (frame->type) {
          case FrameType::Closure: {
            inner_func = frame->closure->Unwrap();
            break;
          }
          case FrameType::Function: {
            inner_func = &frame->function->as.function;
            break;
          }
        }

        auto func_obj = this->Pop();
        // auto func_val = inner_func->chunk.locals[idx];

        Assert(func_obj.type == ValueType::Object);
        auto obj = func_obj.as.object;

        Assert(obj->type == ObjectType::Function);
        auto function = static_cast<Object::Function*>(obj);

        auto closure_idx = this->object_pool->Alloc();
        auto closure = static_cast<Object::Closure*>(this->object_pool->Nth(closure_idx));
        closure->Init(function);

        this->Push(Value(closure));

        for (int i = 0; i < closure->as.closure.upvalue_count; i++) {
          u8 local = READ_BYTE();
          u8 index = READ_BYTE();
          if (local) {
            closure->as.closure.upvalues[i] = this->CaptureUpvalue(frame->locals + index);
          } else {
            // so if its in a nested closure, this check should always be true...
            closure->as.closure.upvalues[i] = frame->closure->as.closure.upvalues[index];
          }
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

  return this->Peek();
}

fnc inline VirtualMachine::CaptureUpvalue(Value* local) -> Object::Upvalue* {
  auto index = this->object_pool->Alloc();
  auto obj = static_cast<Object::Upvalue*>(this->object_pool->Nth(index));
  obj->type = ObjectType::Upvalue;
  obj->as.upvalue.location = local;

  return obj;
}


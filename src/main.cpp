#include <cstdbool>
#include <iostream>
#include <memory>

#include "arena.h"
#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "dynamic_array.h"
#include "global_pool.h"
#include "object.h"
#include "roc_config.h"
#include "utils.h"
#include "vm.h"

#define DEBUG_TRACE_EXECUTION
#undef DEBUG_TRACE_EXECUTION

static VirtualMachine VIRTUAL_MACHINE;
static Compiler COMPILER;

fnc static RunFile(const char* path) -> InterpretResult {
  char* src = Utils::ReadFile(path);
  defer(free(src));

  StringPool string_pool;
  Arena<Object> object_pool;
  GlobalPool global_pool;
  global_pool.Init(&object_pool);

  COMPILER.Init(src, &string_pool, &global_pool);
  const CompileResult compile_res = COMPILER.Compile();
  if (compile_res.IsError()) {
    return InterpretError::CompileError;
  }

  Assert(compile_res.Get().type == ObjectType::Function);
  auto *function = static_cast<Object::Function*>(compile_res.Get());
  VIRTUAL_MACHINE.Init();

  return VIRTUAL_MACHINE.Interpret(function, &string_pool, &object_pool);
}

fnc static Repl() -> void {
  char line[1024];
  InterpretResult status;
  StringPool string_pool;
  Arena<Object> object_pool;
  GlobalPool global_pool;
  global_pool.Init(&object_pool);

  while (true) {
    printf("> ");

    if (fgets(line, sizeof(line), stdin) == nullptr) {
      printf("\n");
      break;
    }

    // @FIXME(eddie) - should reuse the same chunk for this
    COMPILER.Init(line, &string_pool, &global_pool);
    const CompileResult compile_res = COMPILER.Compile();
    if (compile_res.IsError()) {
      status = InterpretError::CompileError;
    } else {
      // @TODO(eddie) - do something with the status
      Assert(compile_res.Get().type == ObjectType::Function);
      auto *function = static_cast<Object::Function*>(compile_res.Get());
      status = VIRTUAL_MACHINE.Interpret(function, &string_pool, &object_pool);
    }
  }
}

fnc main(int argc, char** argv) -> int {
  std::cout << argv[0] << " Version " << Roc_VERSION_MAJOR << "."
            << Roc_VERSION_MINOR << std::endl;

  VIRTUAL_MACHINE.Init();
  defer(VIRTUAL_MACHINE.Deinit());

  if (argc == 1) {
    Repl();
  } else if (argc == 2) {
    RunFile(argv[1]);
  } else {
    printf("Usage: roc [path]\n");
  }

  return 0;
}

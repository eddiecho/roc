#include <iostream>

#include "arena.h"
#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "dynamic_array.h"
#include "roc_config.h"
#include "object.h"
#include "utils.h"
#include "vm.h"

#define DEBUG_TRACE_EXECUTION
#undef DEBUG_TRACE_EXECUTION

static VirtualMachine VM;
static Compiler COMPILER;

fnc static RunFile(const char* path) -> InterpretError {
  char* src = Utils::ReadFile(path);
  defer(free(src));

  Chunk chunk;
  StringPool string_pool;
  Arena<Object> object_pool;

  COMPILER.Init(src, &chunk, &string_pool);
  COMPILER.Compile();
  VM.Init();

  return VM.Interpret(&chunk, &string_pool, &object_pool);
}

fnc static Repl() -> void {
  char line[1024];
  InterpretError status;
  StringPool string_pool;
  Arena<Object> object_pool;

  while (1) {
    Chunk chunk;
    printf("> ");

    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }

    COMPILER.Init(line, &chunk, &string_pool);
    COMPILER.Compile();
    // @TODO(eddie) - do something with the status
    status = VM.Interpret(&chunk, &string_pool, &object_pool);
  }
}

fnc main(int argc, char** argv) -> int {
  std::cout << argv[0] << " Version " << Roc_VERSION_MAJOR << "."
            << Roc_VERSION_MINOR << std::endl;

  VM.Init();
  defer(VM.Deinit());

  if (argc == 1) {
    Repl();
  } else if (argc == 2) {
    RunFile(argv[1]);
  } else {
    printf("Usage: roc [path]\n");
  }

  return 0;
}

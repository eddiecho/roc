#include <iostream>

#include "chunk.cpp"
#include "common.h"
#include "compiler.cpp"
#include "roc_config.h"
#include "utils/utils.h"
#include "vm.cpp"

static VirtualMachine VM;
static Compiler COMPILER;

auto static RunFile(const char* path) -> InterpretError {
  char* src = Utils::ReadFile(path);
  defer(free(src));

  Chunk chunk;
  COMPILER.Init(src, &chunk);
  COMPILER.Compile();

  return VM.Interpret(&chunk);
}

// @TODO(eddie) - it doesnt work because the Interpret() call doesnt work
auto static Repl() -> void {
  char line[1024];
  Chunk chunk;
  InterpretError status;

  while (1) {
    printf("> ");

    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }

    COMPILER.Init(line, &chunk);
    COMPILER.Compile();
    // @TODO(eddie) - do something with the status
    status = VM.Interpret(&chunk);

    chunk.Deinit();
  }
}

int main(int argc, char** argv) {
  std::cout << argv[0] << " Version "
    << Roc_VERSION_MAJOR << "." << Roc_VERSION_MINOR
    << std::endl;

  VM.Init();
  defer(VM.Deinit());

  if (argc == 0) {
    Repl();
  } else if (argc == 2) {
    RunFile(argv[1]);
  } else {
    printf("Usage: roc [path]\n");
  }

  return 0;
}

#include <iostream>

#include "chunk.cpp"
#include "common.h"
#include "roc_config.h"
#include "utils/utils.h"
#include "vm.cpp"

static VirtualMachine VM;

auto static RunFile(const char* path) -> InterpretError {
  char* src = Utils::ReadFile(path);
  defer(free(src));

  return VM.Interpret(src);
}

auto static Repl() -> void {
  char line[1024];

  while (1) {
    printf("> ");

    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }

    VM.Interpret(line);
  }
}

int main(int argc, char** argv) {
  std::cout << argv[0] << " Version "
    << Roc_VERSION_MAJOR << "." << Roc_VERSION_MINOR
    << std::endl;

  VM.Init();
  defer(VM.Deinit(););

  /*
  Chunk chunk;

  chunk.addConstant(1.2, 123);
  chunk.addConstant(42, 123);

  chunk.addConstant(3.4, 123);
  chunk.addChunk(OpCode::Add, 123);
  chunk.addConstant(5.6, 123);
  chunk.addChunk(OpCode::Divide, 123);

  chunk.addChunk(OpCode::Negate, 123);
  chunk.addChunk(OpCode::Return, 199);

  InterpretError error = VM.interpret(&chunk);
  printInterpretError(error);
  */

  if (argc == 0) {
    Repl();
  } else if (argc == 2) {
    RunFile(argv[1]);
  } else {
    printf("Usage: roc [path]\n");
  }

  return 0;
}


#include <iostream>

#include "chunk.cpp"
#include "common.h"
#include "roc_config.h"
#include "utils/utils.h"
#include "vm.cpp"

static VirtualMachine VM;

int main(int argc, char** argv) {
  std::cout << argv[0] << " Version "
    << Roc_VERSION_MAJOR << "." << Roc_VERSION_MINOR
    << std::endl;

  VM.init();

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

  VM.deinit();

  return 0;
}


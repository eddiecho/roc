#include <iostream>

#include "chunk.cpp"
#include "common.h"
#include "roc_config.h"
#include "utils/utils.h"

int main(int argc, char** argv) {
  std::cout << argv[0] << " Version "
    << Roc_VERSION_MAJOR << "." << Roc_VERSION_MINOR
    << std::endl;

  Chunk chunk;

  chunk.addConstant(1.2, 123);
  chunk.addConstant(42, 123);
  chunk.addChunk(OpCode::RETURN, 199);

  chunk.disassemble();
  return 0;
}

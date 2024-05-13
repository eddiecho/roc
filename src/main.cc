#include <stdio.h>
#include <string.h>

#include "token.h"
#include "utils.h"

using namespace roc;

func main(int argc, const char** argv) noexcept -> int {
  switch (argc) {
    case 1: {
      printf("Hello, world!\n");
      return 0;
    }
    case 2: {
      if (strcmp(argv[1], "version") == 0) {
        printf("Hello, world!\n");
        return 0;
      }
    }
  }

  return 0;
}

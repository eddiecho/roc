#include <cstdio>
#include <gtest/gtest.h>

#include "arena.h"
#include "chunk.h"
#include "compiler.h"
#include "utils.h"
#include "value.h"
#include "vm.h"

#ifdef _WIN32
#define TEST_DIR "../../../../test"
#else
#define TEST_DIR "../../test"
#endif

#define MAX_PATH_LEN 256

#define GetTestFilePath(pp) \
  std::snprintf(path, MAX_PATH_LEN, "%s/" pp, TEST_DIR)

class VirtualMachineTest : public ::testing::Test {
 protected:
  void SetUp() override {
    virtual_machine.Init();
  }

  void TearDown() override {
    chunk.Deinit();
    string_pool.Clear();
    virtual_machine.Deinit();
  }

  VirtualMachine virtual_machine;
  Compiler compiler;
  Chunk chunk;
  Arena<char> string_pool;
};

TEST_F(VirtualMachineTest, BasicCompiler) {
  char path[MAX_PATH_LEN];
  GetTestFilePath("scripts/simple1.roc");
  char* src = Utils::ReadFile(path);

  compiler.Init(src, &chunk, &string_pool);
  compiler.Compile();

  InterpretError status = virtual_machine.Interpret(&chunk);
  EXPECT_EQ(status, InterpretError::Success);

  Value val = virtual_machine.Peek();
  EXPECT_EQ(val.type, ValueType::Number);
  EXPECT_DOUBLE_EQ(val.as.number, 7.0);
}

TEST(HelloTest, BasicAssert) {
  char path[MAX_PATH_LEN];
  GetTestFilePath("scripts/simple1.roc");

  char* src = Utils::ReadFile(path);
  EXPECT_EQ(src[0], '(');
}

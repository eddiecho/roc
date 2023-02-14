#include <cstdio>
#include <gtest/gtest.h>

#include "arena.h"
#include "chunk.h"
#include "compiler.h"
#include "dynamic_array.h"
#include "global_pool.h"
#include "object.h"
#include "string_pool.h"
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
    string_pool.Init(&string_object_pool);
    global_pool.Init(&object_pool);
    virtual_machine.Init();
  }

  void TearDown() override {
    chunk.Deinit();
    object_pool.Clear();
    string_pool.Deinit();
    virtual_machine.Deinit();
  }

  VirtualMachine virtual_machine;
  Compiler compiler;
  Chunk chunk;
  StringPool string_pool;
  Arena<Object> string_object_pool;
  Arena<Object> object_pool;
  GlobalPool global_pool;
};

TEST_F(VirtualMachineTest, BasicCompiler) {
  char path[MAX_PATH_LEN];
  GetTestFilePath("scripts/simple1.roc");
  char* src = Utils::ReadFile(path);

  compiler.Init(src, &chunk, &string_pool, &global_pool);
  CompileResult res = compiler.Compile();
  EXPECT_FALSE(res.IsError());

  InterpretError status = virtual_machine.Interpret(res.Get(), &string_pool, &object_pool);
  EXPECT_EQ(status, InterpretError::Success);

  Value val = virtual_machine.Peek();
  EXPECT_EQ(val.type, ValueType::Number);
  EXPECT_DOUBLE_EQ(val.as.number, 7.0);
}

TEST_F(VirtualMachineTest, BasicString) {
  char path[MAX_PATH_LEN];
  GetTestFilePath("scripts/simple_string1.roc");
  char* src = Utils::ReadFile(path);

  compiler.Init(src, &chunk, &string_pool, &global_pool);
  CompileResult res = compiler.Compile();
  EXPECT_FALSE(res.IsError());

  InterpretError status = virtual_machine.Interpret(res.Get(), &string_pool, &object_pool);
  EXPECT_EQ(status, InterpretError::Success);

  Value val = virtual_machine.Peek();
  EXPECT_EQ(val.type, ValueType::Object);
}

TEST_F(VirtualMachineTest, BasicAssignment) {
  char path[MAX_PATH_LEN];
  GetTestFilePath("scripts/simple_assignment.roc");
  char* src = Utils::ReadFile(path);

  compiler.Init(src, &chunk, &string_pool, &global_pool);
  CompileResult res = compiler.Compile();
  EXPECT_FALSE(res.IsError());

  InterpretError status = virtual_machine.Interpret(res.Get(), &string_pool, &object_pool);
  EXPECT_EQ(status, InterpretError::Success);
}

TEST_F(VirtualMachineTest, LocalAssignment) {
  char path[MAX_PATH_LEN];
  GetTestFilePath("scripts/local_assignment.roc");
  char* src = Utils::ReadFile(path);

  compiler.Init(src, &chunk, &string_pool, &global_pool);
  CompileResult res = compiler.Compile();
  EXPECT_FALSE(res.IsError());

  InterpretError status = virtual_machine.Interpret(res.Get(), &string_pool, &object_pool);
  EXPECT_EQ(status, InterpretError::Success);
}

TEST(HelloTest, BasicAssert) {
  char path[MAX_PATH_LEN];
  GetTestFilePath("scripts/simple1.roc");

  char* src = Utils::ReadFile(path);
  EXPECT_EQ(src[0], '(');
}

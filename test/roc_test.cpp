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

fnc Fibonacci(u64 n) -> u64 {
  if (n <= 1) return 1;

  return Fibonacci(n - 1) + Fibonacci(n - 2);
}

class VirtualMachineTest : public ::testing::Test {
 protected:
  void SetUp() override {
    string_pool.Init(&string_object_pool);
    global_pool.Init(&object_pool);
    virtual_machine.Init();
  }

  void TearDown() override {
    object_pool.Clear();
    string_pool.Deinit();
    virtual_machine.Deinit();
  }

  void InitCompiler(const char* test_file) {
    std::snprintf(path, MAX_PATH_LEN, "%s/%s", TEST_DIR, test_file);
    char* src = Utils::ReadFile(path);
    compiler.Init(src, &string_pool, &global_pool);
  }

  char path[MAX_PATH_LEN];

  VirtualMachine virtual_machine;
  Compiler compiler;
  StringPool string_pool;
  Arena<Object> string_object_pool;
  Arena<Object> object_pool;
  GlobalPool global_pool;
};

TEST_F(VirtualMachineTest, BasicCompiler) {
  InitCompiler("scripts/simple1.roc");
  auto res = compiler.Compile();
  EXPECT_FALSE(res.IsError());

  auto function = res.Get();
  auto status = virtual_machine.Interpret(function, &string_pool, &object_pool);
  EXPECT_FALSE(status.IsError());

  auto val = status.Get();
  EXPECT_EQ(val.type, ValueType::Number);
  EXPECT_DOUBLE_EQ(val.as.number, 7.0);
}

TEST_F(VirtualMachineTest, BasicString) {
  InitCompiler("scripts/simple_string1.roc");
  auto res = compiler.Compile();
  EXPECT_FALSE(res.IsError());

  auto function = res.Get();
  auto status = virtual_machine.Interpret(function, &string_pool, &object_pool);
  EXPECT_FALSE(status.IsError());

  auto val = status.Get();
  EXPECT_EQ(val.type, ValueType::Object);
}

TEST_F(VirtualMachineTest, BasicAssignment) {
  InitCompiler("scripts/simple_assignment.roc");
  auto res = compiler.Compile();
  EXPECT_FALSE(res.IsError());

  auto function = res.Get();
  auto status = virtual_machine.Interpret(function, &string_pool, &object_pool);
  EXPECT_FALSE(status.IsError());
}

TEST_F(VirtualMachineTest, LocalAssignment) {
  InitCompiler("scripts/local_assignment.roc");
  auto res = compiler.Compile();
  EXPECT_FALSE(res.IsError());

  auto function = res.Get();
  auto status = virtual_machine.Interpret(function, &string_pool, &object_pool);
  EXPECT_FALSE(status.IsError());
}

TEST_F(VirtualMachineTest, SimpleFunction) {
  InitCompiler("scripts/simple_function.roc");
  auto res = compiler.Compile();
  EXPECT_FALSE(res.IsError());

  auto function = res.Get();
  auto status = virtual_machine.Interpret(function, &string_pool, &object_pool);
  EXPECT_FALSE(status.IsError());

  auto val = status.Get();
  EXPECT_EQ(val.as.number, 27.0);
}

TEST_F(VirtualMachineTest, SimpleRecursion) {
  InitCompiler("scripts/simple_recursion.roc");
  auto res = compiler.Compile();
  EXPECT_FALSE(res.IsError());

  auto function = res.Get();
  auto status = virtual_machine.Interpret(function, &string_pool, &object_pool);
  EXPECT_FALSE(status.IsError());

  auto val = status.Get();
  EXPECT_EQ((u64)val.as.number, Fibonacci(20));
}

TEST_F(VirtualMachineTest, SimpleClosure) {
  InitCompiler("scripts/simple_closure.roc");
  auto res = compiler.Compile();
  EXPECT_FALSE(res.IsError());

  auto function = res.Get();
  auto status = virtual_machine.Interpret(function, &string_pool, &object_pool);
  EXPECT_FALSE(status.IsError());

  auto val = status.Get();
  EXPECT_EQ(val.as.number, 2.0);
}

TEST(HelloTest, BasicAssert) {
  char path[MAX_PATH_LEN];
  GetTestFilePath("scripts/simple1.roc");

  char* src = Utils::ReadFile(path);
  EXPECT_EQ(src[0], '(');
}

#include <cstdio>
#include <gtest/gtest.h>

#include <utility>

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

  u64 prev = 1;
  u64 curr = 1;
  for (u64 i = 1; i < n; i++) {
    prev += curr;
    std::swap(prev, curr);
  }

  return curr;
}

class VirtualMachineTest : public ::testing::Test {
 protected:
  fnc SetUp() -> void override {
    string_pool.Init(&string_object_pool);
    global_pool.Init(&object_pool);
    virtual_machine.Init();
  }

  fnc TearDown() -> void override {
    object_pool.Clear();
    string_pool.Deinit();
    virtual_machine.Deinit();
  }

  fnc InitCompiler(const char* test_file) -> void {
    std::snprintf(path, MAX_PATH_LEN, "%s/%s", TEST_DIR, test_file);
    char* src = Utils::ReadFile(path);
    compiler.Init(src, &string_pool, &global_pool);
  }

  fnc BasicTest(const char* test_file) -> InterpretResult {
    InitCompiler(test_file);
    auto res = compiler.Compile();
    EXPECT_FALSE(res.IsError());

    auto function = res.Get();
    auto status = virtual_machine.Interpret(function, &string_pool, &object_pool);
    EXPECT_FALSE(status.IsError());

    return status;
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
  auto status = BasicTest("scripts/simple1.roc");
  auto val = status.Get();
  EXPECT_EQ(val.type, ValueType::Number);
  EXPECT_DOUBLE_EQ(val.as.number, 7.0);
}

TEST_F(VirtualMachineTest, BasicString) {
  auto status = BasicTest("scripts/simple_string1.roc");
  auto val = status.Get();
  EXPECT_EQ(val.type, ValueType::Object);
}

TEST_F(VirtualMachineTest, BasicAssignment) {
  auto status = BasicTest("scripts/simple_assignment.roc");
}

TEST_F(VirtualMachineTest, LocalAssignment) {
  auto status = BasicTest("scripts/local_assignment.roc");
}

TEST_F(VirtualMachineTest, SimpleFunction) {
  auto status = BasicTest("scripts/simple_function.roc");
  auto val = status.Get();
  EXPECT_EQ(val.as.number, 27.0);
}

TEST_F(VirtualMachineTest, SimpleRecursion) {
  auto status = BasicTest("scripts/simple_recursion.roc");
  auto val = status.Get();
  EXPECT_EQ((u64)val.as.number, Fibonacci(20));
}

TEST_F(VirtualMachineTest, SimpleClosure) {
  auto status = BasicTest("scripts/simple_closure.roc");
  auto val = status.Get();
  EXPECT_EQ(val.as.number, 2.0);
}

TEST(HelloTest, BasicAssert) {
  char path[MAX_PATH_LEN];
  GetTestFilePath("scripts/simple1.roc");

  char* src = Utils::ReadFile(path);
  EXPECT_EQ(src[0], '(');
}

#include <gtest/gtest.h>
#include <cutil/memory.h>
#include <iostream>

#include <tang/tang.h>
#include <tang/macros.h>
#include <tang/computedValue/computedValueAll.h>
#include <tang/program/program.h>
#include <tang/program/bytecode.h>
#include <tang/program/executionContext.h>
#include <tang/program/variable.h>
#include <tang/unicodeString.h>

using namespace std;

#define TEST_REUSABLE_PROGRAM(code) \
  gcu_memory_reset_counts(); \
  GTA_Program * program = gta_program_create(code); \
  ASSERT_TRUE(program); \
  size_t alloc_count = gcu_get_alloc_count(); \
  size_t free_count = gcu_get_free_count();

#define TEST_REUSABLE_PROGRAM_TEARDOWN() \
  gcu_memory_reset_counts(); \
  gta_program_destroy(program); \
  ASSERT_EQ(alloc_count + gcu_get_alloc_count(), free_count + gcu_get_free_count());

#define TEST_CONTEXT_SETUP() \
  gcu_memory_reset_counts(); \
  GTA_Execution_Context * context = gta_execution_context_create(program); \
  ASSERT_TRUE(context);

#define TEST_CONTEXT_TEARDOWN() \
  gta_execution_context_destroy(context); \
  ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());

#define TEST_PROGRAM_SETUP(code) \
  TEST_REUSABLE_PROGRAM(code); \
  TEST_CONTEXT_SETUP(); \
  ASSERT_TRUE(gta_program_execute(context)); \
  ASSERT_TRUE(context->result);

#define TEST_PROGRAM_SETUP_NO_RUN(code) \
  TEST_REUSABLE_PROGRAM(code); \
  TEST_CONTEXT_SETUP();

#define TEST_PROGRAM_TEARDOWN() \
  TEST_CONTEXT_TEARDOWN(); \
  TEST_REUSABLE_PROGRAM_TEARDOWN();


TEST(ControlFlow, IF) {
  {
    // True condition with else branch.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      if (true) {
        print("true");
      } else {
        print("false");
      }
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start true end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // False condition with else branch.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      if (false) {
        print("true");
      } else {
        print("false");
      }
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start false end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // True condition without else branch.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      if (true) {
        print("true");
      }
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start true end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // False condition without else branch.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      if (false) {
        print("true");
      }
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start  end");
    TEST_PROGRAM_TEARDOWN();
  }
}


TEST(ControlFlow, While) {
  {
    // True condition.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      i = 0;
      while (i < 3) {
        print(i);
        i = i + 1;
      }
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 012 end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // False condition.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      i = 3;
      while (i < 3) {
        print(i);
        i = i + 1;
      }
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start  end");
    TEST_PROGRAM_TEARDOWN();
  }
}


TEST(ControlFlow, DoWhile) {
  {
    // True condition.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      i = 0;
      do {
        print(i);
        i = i + 1;
      } while (i < 3);
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 012 end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // False condition.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      i = 3;
      do {
        print(i);
        i = i + 1;
      } while (i < 3);
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 3 end");
    TEST_PROGRAM_TEARDOWN();
  }
}



int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

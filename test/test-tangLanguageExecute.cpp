#include <gtest/gtest.h>
#include <cutil/memory.h>
#include <iostream>

#include <tang/tang.h>
#include <tang/macros.h>
#include <tang/program.h>
#include <tang/executionContext.h>
#include <tang/bytecode.h>
#include <tang/computedValueAll.h>

using namespace std;

TEST(EdgeCase, InvalidSyntax) {
  gcu_memory_reset_counts();
  GTA_Program * program = gta_program_create("invalid syntax :(");
  ASSERT_FALSE(program);
  ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
}

TEST(EdgeCase, Empty) {
  gcu_memory_reset_counts();
  GTA_Program * program = gta_program_create("");
  ASSERT_TRUE(program);
  //gta_program_bytecode_print(program);
  GTA_Execution_Context context;
  ASSERT_TRUE(gta_program_execute(program, &context));
  ASSERT_TRUE(context.result);
  ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context.result));
  gta_program_destroy(program);
  gta_bytecode_execution_context_destroy_in_place(&context);
  ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
}

TEST(SingleValue, Null) {
  gcu_memory_reset_counts();
  GTA_Program * program = gta_program_create("null");
  ASSERT_TRUE(program);
  //gta_program_bytecode_print(program);
  GTA_Execution_Context context;
  ASSERT_TRUE(gta_program_execute(program, &context));
  ASSERT_TRUE(context.result);
  ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context.result));
  gta_program_destroy(program);
  gta_bytecode_execution_context_destroy_in_place(&context);
  ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
}

  // auto p1 = tang->compileScript("3");
  // EXPECT_EQ(*p1.execute().result, (integer_t)3);
  // auto p2 = tang->compileScript("42");
  // EXPECT_EQ(*p2.execute().result, (integer_t)42);
  // auto p3 = tang->compileScript("-42");
  // EXPECT_EQ(*p3.execute().result, (integer_t)-42);
  // auto p4 = tang->compileScript("-42");
  // EXPECT_EQ(*p4.execute().result, (float_t)-42.0);
  // auto p5 = tang->compileScript("-42");
  // EXPECT_NE(*p5.execute().result, (float_t)-42.5);

TEST(Declare, Integer) {
  {
    gcu_memory_reset_counts();
    GTA_Program * program = gta_program_create("3");
    ASSERT_TRUE(program);
    GTA_Execution_Context context;
    ASSERT_TRUE(gta_program_execute(program, &context));
    ASSERT_TRUE(context.result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context.result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context.result)->value, 3);
    gta_program_destroy(program);
    gta_bytecode_execution_context_destroy_in_place(&context);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}


int main(int argc, char **argv) {
  /*
  cout <<
    (GTA_BIG_ENDIAN ? "Big Endian" : "Little Endian") << endl;
  cout <<
    (GTA_32_BIT ? "32-bit" : "64-bit") << endl;
  cout <<
    (GTA_X86 ? "x86" : "Not x86") << endl;
  cout <<
    (GTA_X86_64 ? "x86_64" : "Not x86_64") << endl;
  cout <<
    (GTA_ARM ? "ARM" : "Not ARM") << endl;
  cout <<
    (GTA_ARM64 ? "ARM64" : "Not ARM64") << endl;
  */

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

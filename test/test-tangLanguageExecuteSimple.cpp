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

TEST(Syntax, InvalidSyntax) {
  gcu_memory_reset_counts();
  GTA_Program * program = gta_program_create("invalid syntax :(");
  ASSERT_FALSE(program);
  ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
}

TEST(Syntax, Empty) {
  gcu_memory_reset_counts();
  GTA_Program * program = gta_program_create("");
  ASSERT_TRUE(program);
  GTA_Execution_Context context;
  ASSERT_TRUE(gta_program_execute(program, &context));
  ASSERT_TRUE(context.result);
  ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context.result));
  gta_program_destroy(program);
  gta_bytecode_execution_context_destroy_in_place(&context);
  ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
}

TEST(Declare, Null) {
  gcu_memory_reset_counts();
  GTA_Program * program = gta_program_create("null");
  ASSERT_TRUE(program);
  GTA_Execution_Context context;
  ASSERT_TRUE(gta_program_execute(program, &context));
  ASSERT_TRUE(context.result);
  ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context.result));
  gta_program_destroy(program);
  gta_bytecode_execution_context_destroy_in_place(&context);
  ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
}

TEST(Declare, Boolean) {
  {
    gcu_memory_reset_counts();
    GTA_Program * program = gta_program_create("true");
    ASSERT_TRUE(program);
    GTA_Execution_Context context;
    ASSERT_TRUE(gta_program_execute(program, &context));
    ASSERT_TRUE(context.result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context.result));
    ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context.result)->value);
    gta_program_destroy(program);
    gta_bytecode_execution_context_destroy_in_place(&context);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    gcu_memory_reset_counts();
    GTA_Program * program = gta_program_create("false");
    ASSERT_TRUE(program);
    GTA_Execution_Context context;
    ASSERT_TRUE(gta_program_execute(program, &context));
    ASSERT_TRUE(context.result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context.result));
    ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context.result)->value);
    gta_program_destroy(program);
    gta_bytecode_execution_context_destroy_in_place(&context);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}

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
  {
    gcu_memory_reset_counts();
    GTA_Program * program = gta_program_create("42");
    ASSERT_TRUE(program);
    GTA_Execution_Context context;
    ASSERT_TRUE(gta_program_execute(program, &context));
    ASSERT_TRUE(context.result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context.result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context.result)->value, 42);
    gta_program_destroy(program);
    gta_bytecode_execution_context_destroy_in_place(&context);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    gcu_memory_reset_counts();
    GTA_Program * program = gta_program_create("-42");
    ASSERT_TRUE(program);
    GTA_Execution_Context context;
    ASSERT_TRUE(gta_program_execute(program, &context));
    ASSERT_TRUE(context.result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context.result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context.result)->value, -42);
    gta_program_destroy(program);
    gta_bytecode_execution_context_destroy_in_place(&context);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}

TEST(Declare, Float) {
  {
    gcu_memory_reset_counts();
    GTA_Program * program = gta_program_create("3.14");
    ASSERT_TRUE(program);
    GTA_Execution_Context context;
    ASSERT_TRUE(gta_program_execute(program, &context));
    ASSERT_TRUE(context.result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context.result));
    ASSERT_EQ(((GTA_Computed_Value_Float *)context.result)->value, 3.14);
    gta_program_destroy(program);
    gta_bytecode_execution_context_destroy_in_place(&context);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    gcu_memory_reset_counts();
    GTA_Program * program = gta_program_create("42.0");
    ASSERT_TRUE(program);
    GTA_Execution_Context context;
    ASSERT_TRUE(gta_program_execute(program, &context));
    ASSERT_TRUE(context.result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context.result));
    ASSERT_EQ(((GTA_Computed_Value_Float *)context.result)->value, 42.0);
    gta_program_destroy(program);
    gta_bytecode_execution_context_destroy_in_place(&context);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    gcu_memory_reset_counts();
    GTA_Program * program = gta_program_create("-42.0");
    ASSERT_TRUE(program);
    GTA_Execution_Context context;
    ASSERT_TRUE(gta_program_execute(program, &context));
    ASSERT_TRUE(context.result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context.result));
    ASSERT_EQ(((GTA_Computed_Value_Float *)context.result)->value, -42.0);
    gta_program_destroy(program);
    gta_bytecode_execution_context_destroy_in_place(&context);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    gcu_memory_reset_counts();
    GTA_Program * program = gta_program_create("0.");
    ASSERT_TRUE(program);
    GTA_Execution_Context context;
    ASSERT_TRUE(gta_program_execute(program, &context));
    ASSERT_TRUE(context.result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context.result));
    ASSERT_EQ(((GTA_Computed_Value_Float *)context.result)->value, 0.);
    gta_program_destroy(program);
    gta_bytecode_execution_context_destroy_in_place(&context);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}

TEST(Declare, String) {
  {
    gcu_memory_reset_counts();
    GTA_Program * program = gta_program_create(R"("")");
    ASSERT_TRUE(program);
    GTA_Execution_Context context;
    ASSERT_TRUE(gta_program_execute(program, &context));
    ASSERT_TRUE(context.result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context.result));
    ASSERT_STREQ(((GTA_Computed_Value_String *)context.result)->value->buffer, "");
    gta_program_destroy(program);
    gta_bytecode_execution_context_destroy_in_place(&context);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    gcu_memory_reset_counts();
    GTA_Program * program = gta_program_create(R"("hello")");
    ASSERT_TRUE(program);
    GTA_Execution_Context context;
    ASSERT_TRUE(gta_program_execute(program, &context));
    ASSERT_TRUE(context.result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context.result));
    ASSERT_STREQ(((GTA_Computed_Value_String *)context.result)->value->buffer, "hello");
    gta_program_destroy(program);
    gta_bytecode_execution_context_destroy_in_place(&context);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}

TEST(Declare, Block) {
  if (false) {
    // Empty block.
    gcu_memory_reset_counts();
    GTA_Program * program = gta_program_create("{}");
    ASSERT_TRUE(program);
    GTA_Execution_Context context;
    ASSERT_TRUE(gta_program_execute(program, &context));
    ASSERT_TRUE(context.result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context.result));
    gta_program_destroy(program);
    gta_bytecode_execution_context_destroy_in_place(&context);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  if (false) {
    // Block with a single statement.
    gcu_memory_reset_counts();
    GTA_Program * program = gta_program_create("{3;}");
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
  if (false) {
    // Block with multiple statements.
    gcu_memory_reset_counts();
    GTA_Program * program = gta_program_create("{3; 4;}");
    ASSERT_TRUE(program);
    GTA_Execution_Context context;
    ASSERT_TRUE(gta_program_execute(program, &context));
    ASSERT_TRUE(context.result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context.result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context.result)->value, 4);
    gta_program_destroy(program);
    gta_bytecode_execution_context_destroy_in_place(&context);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Block with multiple statements of different types.
    gcu_memory_reset_counts();
    GTA_Program * program = gta_program_create(R"(3; 4; "foo";)");
    ASSERT_TRUE(program);
    GTA_Execution_Context context;
    ASSERT_TRUE(gta_program_execute(program, &context));
    ASSERT_TRUE(context.result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context.result));
    ASSERT_STREQ(((GTA_Computed_Value_String *)context.result)->value->buffer, "foo");
    gta_program_destroy(program);
    gta_bytecode_execution_context_destroy_in_place(&context);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

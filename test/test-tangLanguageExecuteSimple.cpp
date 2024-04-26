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

#define TEST_PROGRAM_SETUP(code) \
  gcu_memory_reset_counts(); \
  GTA_Program * program = gta_program_create(code); \
  ASSERT_TRUE(program); \
  GTA_Execution_Context * context = gta_execution_context_create(program); \
  ASSERT_TRUE(context); \
  ASSERT_TRUE(gta_program_execute(context)); \
  ASSERT_TRUE(context->result);

#define TEST_PROGRAM_TEARDOWN() \
  gta_execution_context_destroy(context); \
  gta_program_destroy(program); \
  ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());

TEST(Syntax, InvalidSyntax) {
  gcu_memory_reset_counts();
  GTA_Program * program = gta_program_create("invalid syntax :(");
  ASSERT_FALSE(program);
  ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
}

TEST(Syntax, Empty) {
  // An empty program should return a null result.
  TEST_PROGRAM_SETUP("");
  ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
  TEST_PROGRAM_TEARDOWN();
}

TEST(Declare, Null) {
  // The null keyword should return a null result.
  TEST_PROGRAM_SETUP("null");
  ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
  TEST_PROGRAM_TEARDOWN();
}

TEST(Declare, Boolean) {
  {
    // The true keyword should return a boolean result with a value of true.
    TEST_PROGRAM_SETUP("true");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
    ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // The false keyword should return a boolean result with a value of false.
    TEST_PROGRAM_SETUP("false");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
    ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(Declare, Integer) {
  {
    // A positive integer.
    TEST_PROGRAM_SETUP("3");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 3);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // A positive integer.
    TEST_PROGRAM_SETUP("42");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 42);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // A negative integer.
    TEST_PROGRAM_SETUP("-42");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, -42);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // An integer with a value of 0.
    TEST_PROGRAM_SETUP("0");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 0);
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(Declare, Float) {
  {
    // A positive float.
    TEST_PROGRAM_SETUP("3.14");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 3.14);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // A positive float.
    TEST_PROGRAM_SETUP("42.0");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 42.0);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // A negative float.
    TEST_PROGRAM_SETUP("-42.0");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, -42.0);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // A float with a value of 0.
    TEST_PROGRAM_SETUP("0.");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 0.);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // A float with a leading 0.
    TEST_PROGRAM_SETUP("0.42");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 0.42);
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(Declare, String) {
  {
    // Empty string.
    TEST_PROGRAM_SETUP(R"("")");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
    ASSERT_STREQ(((GTA_Computed_Value_String *)context->result)->value->buffer, "");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Non-empty string.
    TEST_PROGRAM_SETUP(R"("hello")");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
    ASSERT_STREQ(((GTA_Computed_Value_String *)context->result)->value->buffer, "hello");
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(Declare, Block) {
  {
    // Empty block.
    TEST_PROGRAM_SETUP("{}");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Block with a single statement.
    TEST_PROGRAM_SETUP("{3;}");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 3);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Block with multiple statements.
    TEST_PROGRAM_SETUP("{3; 4.5;}");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 4.5);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Block with multiple statements of different types.
    TEST_PROGRAM_SETUP(R"(3; 4.5; true; "hello"; "foo";)");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
    ASSERT_STREQ(((GTA_Computed_Value_String *)context->result)->value->buffer, "foo");
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(Assignment, TemporaryLiteral) {
  {
    // Assign a value to a variable.
    TEST_PROGRAM_SETUP("a = 3");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 3);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Assign a value to a variable.
    TEST_PROGRAM_SETUP("a = 3; false;");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Boolean *)context->result)->value, false);
    TEST_PROGRAM_TEARDOWN();
  }
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

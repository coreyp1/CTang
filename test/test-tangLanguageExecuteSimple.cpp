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

#define TEST_PROGRAM_SETUP(code) \
  gcu_memory_reset_counts(); \
  GTA_Program * program = gta_program_create(code); \
  ASSERT_TRUE(program); \
  GTA_Execution_Context * context = gta_execution_context_create(program); \
  ASSERT_TRUE(context); \
  ASSERT_TRUE(gta_program_execute(context)); \
  ASSERT_TRUE(context->result);

#define TEST_PROGRAM_SETUP_NO_RUN(code) \
  gcu_memory_reset_counts(); \
  GTA_Program * program = gta_program_create(code); \
  ASSERT_TRUE(program); \
  GTA_Execution_Context * context = gta_execution_context_create(program); \
  ASSERT_TRUE(context);

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

GTA_Computed_Value * GTA_CALL make_bool_true(GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return (GTA_Computed_Value *)gta_computed_value_boolean_true;
}

GTA_Computed_Value * GTA_CALL make_bool_false(GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return (GTA_Computed_Value *)gta_computed_value_boolean_false;
}

GTA_Computed_Value * GTA_CALL make_int_0(GTA_Execution_Context * context) {
  return (GTA_Computed_Value *)gta_computed_value_integer_create(0, context);
}

GTA_Computed_Value * GTA_CALL make_int_3(GTA_Execution_Context * context) {
  return (GTA_Computed_Value *)gta_computed_value_integer_create(3, context);
}

GTA_Computed_Value * GTA_CALL make_int_42(GTA_Execution_Context * context) {
  return (GTA_Computed_Value *)gta_computed_value_integer_create(42, context);
}

GTA_Computed_Value * GTA_CALL make_float_3_5(GTA_Execution_Context * context) {
  return (GTA_Computed_Value *)gta_computed_value_float_create(3.5, context);
}

GTA_Computed_Value * GTA_CALL make_float_0(GTA_Execution_Context * context) {
  return (GTA_Computed_Value *)gta_computed_value_float_create(0.0, context);
}

GTA_Computed_Value * GTA_CALL make_string_hello(GTA_Execution_Context * context) {
  GTA_Unicode_String * hello = gta_unicode_string_create("hello", 5, GTA_UNICODE_STRING_TYPE_TRUSTED);
  return (GTA_Computed_Value *)gta_computed_value_string_create(hello, true, context);
}

GTA_Computed_Value * GTA_CALL make_string_empty(GTA_Execution_Context * context) {
  GTA_Unicode_String * empty = gta_unicode_string_create("", 0, GTA_UNICODE_STRING_TYPE_TRUSTED);
  return (GTA_Computed_Value *)gta_computed_value_string_create(empty, true, context);
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

TEST(Identifier, Local) {
  {
    // `a` is not declared global, so should not have a value.
    TEST_PROGRAM_SETUP_NO_RUN("a");
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // `a` and `b` are not declared global.
    TEST_PROGRAM_SETUP_NO_RUN("a; b;");
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(Identifier, Library) {
  {
    // `a` is declared global, so should have the default value.
    TEST_PROGRAM_SETUP_NO_RUN("use a; a;");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 3);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // `a` and `b` are libraries.
    // Confirm that `a` can be loaded.
    TEST_PROGRAM_SETUP_NO_RUN("use a; use b; a;");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
    ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_42));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 3);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // `a` and `b` are libraries.
    // Confirm that `b` can be loaded.
    TEST_PROGRAM_SETUP_NO_RUN("use a; use b; b;");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
    ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_42));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 42);
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(Assignment, ToIdentifier) {
  {
    // Assign a value to a local variable.
    TEST_PROGRAM_SETUP("a = 3");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 3);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Assign two local variables, return first.
    TEST_PROGRAM_SETUP("a = 3; b = 4; a;");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 3);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Assign two local variables, return second.
    TEST_PROGRAM_SETUP("a = 3; b = 4; b;");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 4);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Assign a value to a library.
    TEST_PROGRAM_SETUP_NO_RUN("use a; a = 42; a;");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 42);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Load two libraries.  Overwrite the first, return the first.
    TEST_PROGRAM_SETUP_NO_RUN("use a; use b; a = 10; a;");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
    ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_42));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 10);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Load two libraries.  Overwrite the second, return the first.
    TEST_PROGRAM_SETUP_NO_RUN("use a; use b; b = 10; a;");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
    ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_42));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 3);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Load two libraries.  Overwrite the second, return the second.
    TEST_PROGRAM_SETUP_NO_RUN("use a; use b; b = 10; b;");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
    ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_42));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 10);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Load a library, then assign a value to a local variable, then load
    // another library.  Return the local variable.
    // Verifying that the local variable is not overwritten by the library.
    TEST_PROGRAM_SETUP_NO_RUN("use a; b = 10; use c; b;");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
    ASSERT_TRUE(gta_execution_context_add_library(context, "c", make_int_42));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 10);
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(Unary, Negative) {
  // Optimizations mean that something like "-3", which is two AST nodes, will
  // be simplified to a single AST node.  Therefore, we will use a variable to
  // prevent the optimization.
  {
    // Integer negation.
    TEST_PROGRAM_SETUP_NO_RUN("use a; -a;");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, -3);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Double negation.
    TEST_PROGRAM_SETUP_NO_RUN("use a; -(-a);");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 3);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Float negation.
    TEST_PROGRAM_SETUP_NO_RUN("use a; -a;");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, -3.5);
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(Unary, Not) {
  {
    // !null
    TEST_PROGRAM_SETUP_NO_RUN("!a;");
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
    ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // !!null (double negation)
    TEST_PROGRAM_SETUP_NO_RUN("!!a;");
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
    ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Boolean !true
    TEST_PROGRAM_SETUP_NO_RUN("use a; !a;");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_bool_true));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
    ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Boolean !false
    TEST_PROGRAM_SETUP_NO_RUN("use a; !a;");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_bool_false));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
    ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Integer !3
    TEST_PROGRAM_SETUP_NO_RUN("use a; !a;");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
    ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Integer !0
    TEST_PROGRAM_SETUP_NO_RUN("use a; !a;");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_0));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
    ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Float !3.5
    TEST_PROGRAM_SETUP_NO_RUN("use a; !a;");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
    ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Float !0.0
    TEST_PROGRAM_SETUP_NO_RUN("use a; !a;");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_0));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
    ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // String !"hello"
    TEST_PROGRAM_SETUP_NO_RUN("use a; !a;");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_string_hello));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
    ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // String !""
    TEST_PROGRAM_SETUP_NO_RUN("use a; !a;");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_string_empty));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
    ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(Binary, Add) {
  {
    // Integer + Integer.
    TEST_PROGRAM_SETUP_NO_RUN("use a; use b; a + b;");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
    ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_42));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 45);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Integer + Float.
    TEST_PROGRAM_SETUP_NO_RUN("use a; use b; a + b;");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
    ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 6.5);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Float + Float.
    TEST_PROGRAM_SETUP_NO_RUN("use a; use b; a + b;");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
    ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 7.0);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Float + Integer.
    TEST_PROGRAM_SETUP_NO_RUN("use a; use b; a + b;");
    ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
    ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 6.5);
    TEST_PROGRAM_TEARDOWN();
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

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

GTA_Computed_Value * GTA_CALL make_int_negative_10(GTA_Execution_Context * context) {
  return (GTA_Computed_Value *)gta_computed_value_integer_create(-10, context);
}

GTA_Computed_Value * GTA_CALL make_int_42(GTA_Execution_Context * context) {
  return (GTA_Computed_Value *)gta_computed_value_integer_create(42, context);
}

GTA_Computed_Value * GTA_CALL make_float_3(GTA_Execution_Context * context) {
  return (GTA_Computed_Value *)gta_computed_value_float_create(3., context);
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

GTA_Computed_Value * GTA_CALL make_string_3(GTA_Execution_Context * context) {
  GTA_Unicode_String * three = gta_unicode_string_create("3", 1, GTA_UNICODE_STRING_TYPE_TRUSTED);
  return (GTA_Computed_Value *)gta_computed_value_string_create(three, true, context);
}

GTA_Computed_Value * GTA_CALL make_string_42f5(GTA_Execution_Context * context) {
  GTA_Unicode_String * forty_two = gta_unicode_string_create("42.5", 4, GTA_UNICODE_STRING_TYPE_TRUSTED);
  return (GTA_Computed_Value *)gta_computed_value_string_create(forty_two, true, context);
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

TEST(Declare, Array) {
  {
    // Empty array.
    TEST_PROGRAM_SETUP("[]");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ARRAY(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Array *)context->result)->elements->count, 0);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Array with a single element.
    TEST_PROGRAM_SETUP("[3]");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ARRAY(context->result));
    GTA_Computed_Value_Array * array = (GTA_Computed_Value_Array *)context->result;
    ASSERT_EQ(array->elements->count, 1);
    GTA_Computed_Value * value = (GTA_Computed_Value *)GTA_TYPEX_P(array->elements->data[0]);
    ASSERT_FALSE(value->is_temporary);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(value));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)(value))->value, 3);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Array with multiple elements.
    TEST_PROGRAM_SETUP("[3, 4.5, true, \"hello\"]");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ARRAY(context->result));
    GTA_Computed_Value_Array * array = (GTA_Computed_Value_Array *)context->result;
    ASSERT_EQ(array->elements->count, 4);
    GTA_Computed_Value * value = (GTA_Computed_Value *)GTA_TYPEX_P(array->elements->data[0]);
    ASSERT_FALSE(value->is_temporary);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(value));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)(value))->value, 3);
    value = (GTA_Computed_Value *)GTA_TYPEX_P(array->elements->data[1]);
    ASSERT_FALSE(value->is_temporary);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(value));
    ASSERT_EQ(((GTA_Computed_Value_Float *)(value))->value, 4.5);
    value = (GTA_Computed_Value *)GTA_TYPEX_P(array->elements->data[2]);
    ASSERT_FALSE(value->is_temporary);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(value));
    ASSERT_TRUE(((GTA_Computed_Value_Boolean *)(value))->value);
    value = (GTA_Computed_Value *)GTA_TYPEX_P(array->elements->data[3]);
    ASSERT_FALSE(value->is_temporary);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(value));
    ASSERT_STREQ(((GTA_Computed_Value_String *)(value))->value->buffer, "hello");
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
    TEST_REUSABLE_PROGRAM("use a; use b; a + b;");
    {
      // Integer + Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 45);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer + Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 6.5);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float + Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 7.0);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float + Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 6.5);
      TEST_CONTEXT_TEARDOWN();
    }
    TEST_REUSABLE_PROGRAM_TEARDOWN();
  }
}

TEST(Binary, Subtract) {
  {
    TEST_REUSABLE_PROGRAM("use a; use b; a - b;");
    {
      // Integer - Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 39);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer - Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 38.5);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float - Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 0.0);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float - Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 0.5);
      TEST_CONTEXT_TEARDOWN();
    }
    TEST_REUSABLE_PROGRAM_TEARDOWN();
  }
}

TEST(Binary, Multiply) {
  {
    TEST_REUSABLE_PROGRAM("use a; use b; a * b;");
    {
      // Integer * Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 126);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer * Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 10.5);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float * Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 12.25);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float * Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 10.5);
      TEST_CONTEXT_TEARDOWN();
    }
    TEST_REUSABLE_PROGRAM_TEARDOWN();
  }
}

TEST(Binary, Divide) {
  {
    TEST_REUSABLE_PROGRAM("use a; use b; a / b;");
    {
      // Integer / Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 14.0);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer / -Integer. (not evenly divisible)
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_negative_10));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, -3);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer / Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 12.0);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float / Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 1.0);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float / Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 1.1666666666666667);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Division by zero, Integer / Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ERROR(context->result));
      ASSERT_EQ(context->result, gta_computed_value_error_divide_by_zero);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Division by zero, Integer / Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ERROR(context->result));
      ASSERT_EQ(context->result, gta_computed_value_error_divide_by_zero);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Division by zero, Float / Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ERROR(context->result));
      ASSERT_EQ(context->result, gta_computed_value_error_divide_by_zero);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Division by zero, Float / Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ERROR(context->result));
      ASSERT_EQ(context->result, gta_computed_value_error_divide_by_zero);
      TEST_CONTEXT_TEARDOWN();
    }
    TEST_REUSABLE_PROGRAM_TEARDOWN();
  }
}

TEST(Binary, Modulo) {
  {
    TEST_REUSABLE_PROGRAM("use a; use b; a % b;");
    {
      // Integer % Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 0);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // -Integer % Integer. (not evenly divisible)
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_negative_10));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, -1);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer % -Integer. (not evenly divisible)
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_negative_10));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 2);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer % Float. Should error.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ERROR(context->result));
      ASSERT_EQ(context->result, gta_computed_value_error_not_supported);
      TEST_CONTEXT_TEARDOWN();
    }
    TEST_REUSABLE_PROGRAM_TEARDOWN();
  }
}

TEST(Binary, LessThan) {
  {
    TEST_REUSABLE_PROGRAM("use a; use b; a < b;");
    {
      // Integer < Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer < Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer < Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer < Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer < Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float < Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_0));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float < Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float < Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float < Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float < Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    TEST_REUSABLE_PROGRAM_TEARDOWN();
  }
}

TEST(Binary, LessThanEqual) {
  {
    TEST_REUSABLE_PROGRAM("use a; use b; a <= b;");
    {
      // Integer <= Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer <= Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer <= Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer <= Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer <= Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer <= Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float <= Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_0));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float <= Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float <= Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float <= Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float <= Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float <= Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    TEST_REUSABLE_PROGRAM_TEARDOWN();
  }
}

TEST(Binary, GreaterThan) {
  {
    TEST_REUSABLE_PROGRAM("use a; use b; a > b;");
    {
      // Integer > Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer > Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer > Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer > Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer > Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float > Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float > Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float > Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_0));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float > Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float > Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    TEST_REUSABLE_PROGRAM_TEARDOWN();
  }
}

TEST(Binary, GreaterThanEqual) {
  {
    TEST_REUSABLE_PROGRAM("use a; use b; a >= b;");
    {
      // Integer >= Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer >= Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer >= Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer >= Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer >= Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer >= Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float >= Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float >= Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float >= Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_0));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float >= Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float >= Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    TEST_REUSABLE_PROGRAM_TEARDOWN();
  }
}

TEST(Binary, Equal) {
  {
    TEST_REUSABLE_PROGRAM("use a; use b; a == b;");
    {
      // Integer == Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer == Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer == Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer == Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float == Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float == Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float == Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float == Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    TEST_REUSABLE_PROGRAM_TEARDOWN();
  }
}

TEST(Binary, NotEqual) {
  {
    TEST_REUSABLE_PROGRAM("use a; use b; a != b;");
    {
      // Integer != Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer != Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer != Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer != Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float != Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float != Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_float_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float != Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float != Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    TEST_REUSABLE_PROGRAM_TEARDOWN();
  }
}

TEST(Binary, And) {
  {
    TEST_REUSABLE_PROGRAM("use a; use b; a && b;");
    {
      // true && true => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_bool_true));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_bool_true));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // true && false => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_bool_true));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_bool_false));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // false && true => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_bool_false));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_bool_true));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // false && false => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_bool_false));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_bool_false));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    TEST_REUSABLE_PROGRAM_TEARDOWN();
  }
}

TEST(Binary, Or) {
  {
    TEST_REUSABLE_PROGRAM("use a; use b; a || b;");
    {
      // true || true => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_bool_true));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_bool_true));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // true || false => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_bool_true));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_bool_false));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // false || true => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_bool_false));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_bool_true));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // false || false => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_bool_false));
      ASSERT_TRUE(gta_execution_context_add_library(context, "b", make_bool_false));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    TEST_REUSABLE_PROGRAM_TEARDOWN();
  }
}

TEST(Cast, ToBoolean) {
  {
    TEST_REUSABLE_PROGRAM("use a; a as bool;");
    {
      // Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Boolean => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_bool_true));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Null => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // String => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_string_hello));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Empty String => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_string_empty));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    TEST_REUSABLE_PROGRAM_TEARDOWN();
  }
}

TEST(Case, ToInteger) {
  {
    TEST_REUSABLE_PROGRAM("use a; a as int;");
    {
      // Integer => Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(42, ((GTA_Computed_Value_Integer *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float => Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(3, ((GTA_Computed_Value_Integer *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Boolean => Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_bool_true));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(1, ((GTA_Computed_Value_Integer *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Null => Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(0, ((GTA_Computed_Value_Integer *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // String => Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_string_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(3, ((GTA_Computed_Value_Integer *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    TEST_REUSABLE_PROGRAM_TEARDOWN();
  }
}

TEST(Cast, ToFloat) {
  {
    TEST_REUSABLE_PROGRAM("use a; a as float;");
    {
      // Integer => Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(42.0, ((GTA_Computed_Value_Float *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float => Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(3.5, ((GTA_Computed_Value_Float *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Boolean => Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_bool_true));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(1.0, ((GTA_Computed_Value_Float *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Null => Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(0.0, ((GTA_Computed_Value_Float *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // String => Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_string_42f5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(42.5, ((GTA_Computed_Value_Float *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    TEST_REUSABLE_PROGRAM_TEARDOWN();
  }
}

TEST(Cast, ToString) {
  {
    TEST_REUSABLE_PROGRAM("use a; a as string;");
    {
      // Integer => String.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
      ASSERT_STREQ("42", ((GTA_Computed_Value_String *)context->result)->value->buffer);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float => String.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
      ASSERT_STREQ("3.5", ((GTA_Computed_Value_String *)context->result)->value->buffer);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Boolean => String.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_bool_true));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
      ASSERT_STREQ("true", ((GTA_Computed_Value_String *)context->result)->value->buffer);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Null => String.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
      ASSERT_STREQ("null", ((GTA_Computed_Value_String *)context->result)->value->buffer);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // String => String.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_execution_context_add_library(context, "a", make_string_hello));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
      ASSERT_STREQ("hello", ((GTA_Computed_Value_String *)context->result)->value->buffer);
      TEST_CONTEXT_TEARDOWN();
    }
    TEST_REUSABLE_PROGRAM_TEARDOWN();
  }
}

TEST(Print, Simple) {
  {
    // Print an integer.
    TEST_PROGRAM_SETUP("print(42);");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
    ASSERT_STREQ("42", context->output->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Print a float.
    TEST_PROGRAM_SETUP("print(3.5);");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
    ASSERT_STREQ("3.5", context->output->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Print a string.
    TEST_PROGRAM_SETUP("print(\"hello\");");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
    ASSERT_STREQ("hello", context->output->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Print a null (should print nothing).
    TEST_PROGRAM_SETUP("print(null);");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
    ASSERT_STREQ("", context->output->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Multiple prints.
    TEST_PROGRAM_SETUP("print(42); print(3.5); print(\"hello\"); print(null); print(-42);");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
    ASSERT_STREQ("423.5hello-42", context->output->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

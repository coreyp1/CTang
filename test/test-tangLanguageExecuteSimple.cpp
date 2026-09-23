
#include <assert.h>
#include <gtest/gtest.h>
#include <ghoti.io/cutil/memory.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <unicode/uclean.h>

#include <ghoti.io/tang/tang.h>
#include <ghoti.io/tang/macros.h>
#include <ghoti.io/tang/computedValue/computedValueAll.h>
#include <ghoti.io/tang/program/program.h>
#include <ghoti.io/tang/program/bytecode.h>
#include <ghoti.io/tang/program/executionContext.h>
#include <ghoti.io/tang/program/variable.h>
#include <ghoti.io/tang/unicodeString.h>

using namespace std;


GTA_Language * language;


#define TEST_REUSABLE_PROGRAM(code) \
  gcu_memory_reset_counts(); \
  GTA_Program * program = gta_program_create(language, code); \
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
  GTA_Program * program = gta_program_create(language, "invalid syntax :(");
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

TEST(Declare, ManyDistinctLiterals) {
  // Each distinct literal value is interned as a singleton, in a table
  // created with room for 32. Inserting the 33rd grows it, and growth used to
  // run the table's cleanup hook over the entries it was moving rather than
  // discarding - destroying the very singletons the compiled program was
  // about to use. A program with more than 32 distinct literals died there,
  // as a segfault or a heap-corruption abort depending on what reused the
  // freed block.
  //
  // Nothing in this suite had crossed that threshold, which is why it went
  // unnoticed; these cases sit either side of it.
  for (int count : {32, 33, 100}) {
    std::string code = "a = 0;";
    for (int i = 0; i < count; ++i) {
      code += " a = " + std::to_string(i) + ";";
    }
    code += " a;";

    TEST_PROGRAM_SETUP(code.c_str());
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result))
        << count << " distinct literals";
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value,
        count - 1)
        << count << " distinct literals";
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

TEST(Declare, Map) {
  {
    // Empty map.
    TEST_PROGRAM_SETUP("{:}");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_MAP(context->result));
    GTA_Computed_Value_Map * map = (GTA_Computed_Value_Map *)context->result;
    ASSERT_EQ(map->key_hash->entries, 0);
    ASSERT_EQ(map->value_hash->entries, 0);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Map with multiple key-value pairs.
    TEST_PROGRAM_SETUP(R"({number: 4.5, greeting: "hello"})");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_MAP(context->result));
    GTA_Computed_Value_Map * map = (GTA_Computed_Value_Map *)context->result;
    ASSERT_EQ(map->key_hash->entries, 2);
    ASSERT_EQ(map->value_hash->entries, 2);
    GTA_Computed_Value * val1 = gta_computed_value_map_get_from_cstring(map, "number");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(val1));
    ASSERT_EQ(((GTA_Computed_Value_Float *)val1)->value, 4.5);
    GTA_Computed_Value * val2 = gta_computed_value_map_get_from_cstring(map, "greeting");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(val2));
    ASSERT_STREQ(((GTA_Computed_Value_String *)val2)->value->buffer, "hello");
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
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
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
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_42));
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
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_42));
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
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 42);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Load two libraries.  Overwrite the first, return the first.
    TEST_PROGRAM_SETUP_NO_RUN("use a; use b; a = 10; a;");
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_42));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 10);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Load two libraries.  Overwrite the second, return the first.
    TEST_PROGRAM_SETUP_NO_RUN("use a; use b; b = 10; a;");
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_42));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 3);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Load two libraries.  Overwrite the second, return the second.
    TEST_PROGRAM_SETUP_NO_RUN("use a; use b; b = 10; b;");
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_42));
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
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "c", make_int_42));
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
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, -3);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Double negation.
    TEST_PROGRAM_SETUP_NO_RUN("use a; -(-a);");
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 3);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Float negation.
    TEST_PROGRAM_SETUP_NO_RUN("use a; -a;");
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
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
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_bool_true));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
    ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Boolean !false
    TEST_PROGRAM_SETUP_NO_RUN("use a; !a;");
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_bool_false));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
    ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Integer !3
    TEST_PROGRAM_SETUP_NO_RUN("use a; !a;");
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
    ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Integer !0
    TEST_PROGRAM_SETUP_NO_RUN("use a; !a;");
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_0));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
    ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Float !3.5
    TEST_PROGRAM_SETUP_NO_RUN("use a; !a;");
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
    ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Float !0.0
    TEST_PROGRAM_SETUP_NO_RUN("use a; !a;");
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_0));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
    ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // String !"hello"
    TEST_PROGRAM_SETUP_NO_RUN("use a; !a;");
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_string_hello));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
    ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // String !""
    TEST_PROGRAM_SETUP_NO_RUN("use a; !a;");
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_string_empty));
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
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 45);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer + Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 6.5);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float + Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 7.0);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float + Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
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
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 39);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer - Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 38.5);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float - Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 0.0);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float - Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
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
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 126);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer * Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 10.5);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float * Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 12.25);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float * Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
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
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 14.0);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer / -Integer. (not evenly divisible)
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_negative_10));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, -3);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer / Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 12.0);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float / Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 1.0);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float / Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Float *)context->result)->value, 1.1666666666666667);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Division by zero, Integer / Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ERROR(context->result));
      ASSERT_EQ(context->result, gta_computed_value_error_divide_by_zero);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Division by zero, Integer / Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ERROR(context->result));
      ASSERT_EQ(context->result, gta_computed_value_error_divide_by_zero);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Division by zero, Float / Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ERROR(context->result));
      ASSERT_EQ(context->result, gta_computed_value_error_divide_by_zero);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Division by zero, Float / Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_0));
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
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 0);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // -Integer % Integer. (not evenly divisible)
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_negative_10));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, -1);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer % -Integer. (not evenly divisible)
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_negative_10));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(((GTA_Computed_Value_Integer *)context->result)->value, 2);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer % Float. Should error.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
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
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer < Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer < Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer < Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer < Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float < Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_0));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float < Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float < Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float < Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float < Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
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
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer <= Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer <= Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer <= Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer <= Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer <= Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float <= Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_0));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float <= Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float <= Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float <= Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float <= Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float <= Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
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
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer > Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer > Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer > Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer > Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float > Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float > Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float > Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_0));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float > Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float > Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_42));
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
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer >= Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer >= Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer >= Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer >= Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer >= Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float >= Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float >= Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float >= Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_0));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float >= Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float >= Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_42));
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
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer == Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer == Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer == Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float == Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float == Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float == Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float == Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
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
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer != Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer != Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer != Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float != Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float != Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_float_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float != Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float != Integer => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_3));
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
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_bool_true));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_bool_true));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // true && false => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_bool_true));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_bool_false));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // false && true => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_bool_false));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_bool_true));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // false && false => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_bool_false));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_bool_false));
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
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_bool_true));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_bool_true));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // true || false => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_bool_true));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_bool_false));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // false || true => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_bool_false));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_bool_true));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // false || false => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_bool_false));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_bool_false));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    TEST_REUSABLE_PROGRAM_TEARDOWN();
  }
}

TEST(Ternary, Conditional) {
  {
    TEST_REUSABLE_PROGRAM("use a; use b; use c; a ? b : c;");
    {
      // true ? 42 : 3 => 42.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_bool_true));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "c", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(42, ((GTA_Computed_Value_Integer *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // false ? 42 : 3 => 3.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_bool_false));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "b", make_int_42));
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "c", make_int_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(3, ((GTA_Computed_Value_Integer *)context->result)->value);
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
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Integer => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_0));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Boolean => true.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_bool_true));
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
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_string_hello));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
      ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Empty String => false.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_string_empty));
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
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(42, ((GTA_Computed_Value_Integer *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float => Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(3, ((GTA_Computed_Value_Integer *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Boolean => Integer.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_bool_true));
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
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_string_3));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
      ASSERT_EQ(3, ((GTA_Computed_Value_Integer *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    TEST_REUSABLE_PROGRAM_TEARDOWN();
  }
}

TEST(Syntax, IntegerDivisionOverflowDoesNotKillTheProcess) {
  // INT64_MIN / -1 overflows, and on x86-64 the hardware raises SIGFPE rather
  // than wrapping - so this was not a wrong answer but a dead process, from a
  // one-line script, in both execution engines.
  //
  // This test only asserts that the line is reached at all. What the operators
  // now answer is asserted in Syntax.IntegerOverflowIsReportedNotWrapped.
  {
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      print((0 - 9223372036854775807 - 1) / -1);
      print(" end");
    )");
    // What matters is reaching this line at all.
    ASSERT_NE(context->output->buffer, nullptr);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      print((0 - 9223372036854775807 - 1) % -1);
      print(" end");
    )");
    ASSERT_NE(context->output->buffer, nullptr);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Ordinary division must be untouched by the guard.
    TEST_PROGRAM_SETUP(R"(
      print(10 / 2); print(","); print(10 % 3); print(","); print(0 - 7 / 2);
    )");
    ASSERT_STREQ(context->output->buffer, "5,1,-3");
    TEST_PROGRAM_TEARDOWN();
  }
}


TEST(Cast, OutOfRangeStringToIntegerSaysSo) {
  // `"99999999999999999999999" as int` was 9223372036854775807 - the clamp to
  // INT_MAX that the markers exist to remove - and it got there through atoll,
  // whose behaviour when the value does not fit is undefined. glibc happens to
  // saturate; nothing promises it.
  //
  // The marker text is the same as the float cast's deliberately. Both mean
  // "this value does not fit in an integer", and spelling them differently by
  // source type would invite a reader to think they were different conditions.
  {
    TEST_PROGRAM_SETUP(R"(print("99999999999999999999999" as int);)");
    ASSERT_STREQ(context->output->buffer, "[INTEGER TOO LARGE]");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    TEST_PROGRAM_SETUP(R"(print("-99999999999999999999999" as int);)");
    ASSERT_STREQ(context->output->buffer, "[INTEGER TOO SMALL]");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // The boundaries themselves convert; one past does not.
    TEST_PROGRAM_SETUP(R"(print("9223372036854775807" as int); print(","); print("-9223372036854775808" as int);)");
    ASSERT_STREQ(context->output->buffer, "9223372036854775807,-9223372036854775808");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    TEST_PROGRAM_SETUP(R"(print("9223372036854775808" as int);)");
    ASSERT_STREQ(context->output->buffer, "[INTEGER TOO LARGE]");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // The leading-number rule is untouched: a string that continues past its
    // number still converts, and leading whitespace is still skipped.
    TEST_PROGRAM_SETUP(R"(print("42" as int); print(","); print("12abc" as int); print(","); print("  7" as int);)");
    ASSERT_STREQ(context->output->buffer, "42,12,7");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // A string with no leading number is not 0. It used to be, which made it
    // indistinguishable from the string "0" - two different situations giving
    // one plausible answer, which is the collapse these markers undo.
    TEST_PROGRAM_SETUP(R"(print("abc" as int); print(","); print("" as int); print(","); print("0" as int);)");
    ASSERT_STREQ(context->output->buffer, "[NOT A NUMBER],[NOT A NUMBER],0");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // The float cast answers the same way, and for the same reason.
    TEST_PROGRAM_SETUP(R"(print("abc" as float); print(","); print("" as float); print(","); print("3.3" as float); print(","); print("12abc" as float);)");
    ASSERT_STREQ(context->output->buffer, "[NOT A NUMBER],[NOT A NUMBER],3.3,12.");
    TEST_PROGRAM_TEARDOWN();
  }
}


TEST(Cast, OutOfRangeFloatToIntegerSaysSo) {
  // Casting a float that does not fit in an integer used to be undefined
  // behaviour, and on x86-64 the conversion instruction answers with the
  // "integer indefinite" value. That is GTA_INTEGER_MIN - for a huge POSITIVE
  // float as readily as a huge negative one - so `hugeFloat as int` printed
  // -9223372036854775808 and nothing told the reader that the number was not
  // the answer. It now says which way it went out of range.
  //
  // Both engines run this file, and both must agree: the constant folder
  // declines to fold an out-of-range cast rather than carrying a second copy
  // of the rule, so there is one answer by construction.

  // Larger than any double, so it is an infinity. Exponent notation does not
  // lex, hence the digits.
  const std::string huge = std::string(320, '9') + ".0";

  {
    // In range, and still folded and converted as before.
    TEST_PROGRAM_SETUP(R"(print(3.9 as int); print(","); print((0.0 - 3.9) as int);)");
    ASSERT_STREQ(context->output->buffer, "3,-3");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Finite, but far outside the range.
    TEST_PROGRAM_SETUP(R"(print(77777777777777777777777.9 as int);)");
    ASSERT_STREQ(context->output->buffer, "[INTEGER TOO LARGE]");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    TEST_PROGRAM_SETUP(R"(print((0.0 - 77777777777777777777777.9) as int);)");
    ASSERT_STREQ(context->output->buffer, "[INTEGER TOO SMALL]");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Infinity.
    std::string src = "print(" + huge + " as int);";
    TEST_PROGRAM_SETUP(src.c_str());
    ASSERT_STREQ(context->output->buffer, "[INTEGER TOO LARGE]");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    std::string src = "print((0.0 - " + huge + ") as int);";
    TEST_PROGRAM_SETUP(src.c_str());
    ASSERT_STREQ(context->output->buffer, "[INTEGER TOO SMALL]");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // A NaN is neither, and is reachable: infinity minus infinity. It compares
    // false against every bound, so without its own test it would fall through
    // into the conversion and be undefined behaviour again.
    std::string src = "print((" + huge + " - " + huge + ") as int);";
    TEST_PROGRAM_SETUP(src.c_str());
    ASSERT_STREQ(context->output->buffer, "[NOT A NUMBER]");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // The boundary, which is where a naive test gets it wrong. GTA_INTEGER_MAX
    // is not representable as a double and rounds UP to 2^63, so the literal
    // spelling of the maximum does not fit and must be refused...
    TEST_PROGRAM_SETUP(R"(print(9223372036854775807.0 as int);)");
    ASSERT_STREQ(context->output->buffer, "[INTEGER TOO LARGE]");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // ...while the largest double below 2^63 converts exactly, and so does
    // GTA_INTEGER_MIN, which is a power of two and exact.
    TEST_PROGRAM_SETUP(R"(print(9223372036854774784.0 as int); print(","); print((0.0 - 9223372036854775808.0) as int);)");
    ASSERT_STREQ(context->output->buffer, "9223372036854774784,-9223372036854775808");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // The marker prints where the number would have gone, and execution
    // carries on - it is a value, not a halt.
    TEST_PROGRAM_SETUP(R"(print("A"); print(77777777777777777777777.9 as int); print("B");)");
    ASSERT_STREQ(context->output->buffer, "A[INTEGER TOO LARGE]B");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // It survives assignment, so it cannot be laundered into a number by
    // storing it first.
    TEST_PROGRAM_SETUP(R"(x = 77777777777777777777777.9 as int; print(x);)");
    ASSERT_STREQ(context->output->buffer, "[INTEGER TOO LARGE]");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // And it is an error value: arithmetic on it does not produce a number,
    // and it is false in a condition.
    TEST_PROGRAM_SETUP(R"(if (77777777777777777777777.9 as int) { print("T"); } else { print("F"); })");
    ASSERT_STREQ(context->output->buffer, "F");
    TEST_PROGRAM_TEARDOWN();
  }
}


TEST(Print, Boolean) {
  // Printing a boolean rendered as nothing: the boolean vtable's print was
  // print_not_supported, even though its to_string already produced "true" and
  // "false" and `true as string` already printed them. In a language whose job
  // is generating text, that is a value that cannot be seen - and it quietly
  // turned `print(a == b)` in a test into an assertion of nothing.
  {
    TEST_PROGRAM_SETUP(R"(print(true); print(","); print(false);)");
    ASSERT_STREQ(context->output->buffer, "true,false");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // However the boolean arrives: from a comparison, from a negation, from a
    // cast, and through a variable.
    TEST_PROGRAM_SETUP(R"(
      print(1 < 2); print(","); print(1 > 2); print(",");
      print(!true); print(","); print(1 as bool); print(",");
      x = 2 == 2; print(x);
    )");
    ASSERT_STREQ(context->output->buffer, "true,false,false,true,true");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // And it still agrees with the string cast, which was already right.
    TEST_PROGRAM_SETUP(R"(print(true as string); print(","); print(true);)");
    ASSERT_STREQ(context->output->buffer, "true,true");
    TEST_PROGRAM_TEARDOWN();
  }
}


TEST(Syntax, IntegerLiteralOutOfRangeIsRejected) {
  // A literal that does not fit used to saturate, so `9223372036854775808`
  // silently became `9223372036854775807`. A literal has no operands and
  // cannot vary, so there is nothing to wait for: it is refused at parse time
  // rather than answered with a run-time marker.
  for (const char * src : {
      "9223372036854775808;",
      "99999999999999999999999;",
      "9223372036854775808 - 1;",
      "x = 9223372036854775808;",
      "-99999999999999999999999;",
      }) {
    gcu_memory_reset_counts();
    GTA_Program * program = gta_program_create(language, src);
    ASSERT_FALSE(program) << "accepted: " << src;
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count()) << "leaked on: " << src;
  }
  {
    // The bounds themselves must still be accepted.
    TEST_PROGRAM_SETUP(R"(print(9223372036854775807);)");
    ASSERT_STREQ(context->output->buffer, "9223372036854775807");
    TEST_PROGRAM_TEARDOWN();
  }
}


TEST(Syntax, TheMostNegativeIntegerCanBeWritten) {
  // Tang has no negative literals - unary minus is an operator - and the
  // signed range is asymmetric, so the most negative integer can only be
  // written as minus applied to a magnitude that is itself one past the
  // maximum. That made it unwritable twice over: the literal saturated, so
  // `-9223372036854775808` evaluated to `-9223372036854775807`, off by one and
  // silently; and refusing out-of-range literals outright would have left no
  // way to write it at all.
  //
  // So that magnitude is accepted in exactly one position, directly after
  // unary minus, and is a syntax error anywhere else. Java has the same
  // carve-out for the same reason.
  {
    TEST_PROGRAM_SETUP(R"(print(-9223372036854775808);)");
    ASSERT_STREQ(context->output->buffer, "-9223372036854775808");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Whitespace between the operator and the literal is still fine, and the
    // value is exact rather than a negation that would itself overflow - so it
    // matches the arithmetic spelling of the same number. Compared by printing
    // both rather than by printing the comparison, because printing a boolean
    // produces nothing (13.8).
    TEST_PROGRAM_SETUP(R"(print(- 9223372036854775808); print(","); print(0 - 9223372036854775807 - 1);)");
    ASSERT_STREQ(context->output->buffer, "-9223372036854775808,-9223372036854775808");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // But only there. Without the minus it is the out-of-range literal again.
    gcu_memory_reset_counts();
    GTA_Program * program = gta_program_create(language, "9223372036854775808;");
    ASSERT_FALSE(program);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}


TEST(Syntax, IntegerOverflowIsReportedNotWrapped) {
  // Arithmetic that cannot produce a representable answer says so, rather than
  // wrapping. Wrapping was the worst of the three options: a clamp is wrong by
  // a bounded amount, but 9223372036854775807 + 1 came back as the most
  // negative integer there is, which looks exactly like a real result.
  //
  // GTA_INTEGER_MIN cannot be written as a literal - 9223372036854775808
  // saturates to the maximum - so it is built by arithmetic here.
  //
  // Both engines run this file and both must agree: the constant folder
  // declines to fold anything that overflows, so run time answers all of it.
  {
    TEST_PROGRAM_SETUP(R"(print(9223372036854775807 + 1);)");
    ASSERT_STREQ(context->output->buffer, "[INTEGER TOO LARGE]");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    TEST_PROGRAM_SETUP(R"(print((0 - 9223372036854775807 - 1) - 1);)");
    ASSERT_STREQ(context->output->buffer, "[INTEGER TOO SMALL]");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    TEST_PROGRAM_SETUP(R"(print(9223372036854775807 * 2);)");
    ASSERT_STREQ(context->output->buffer, "[INTEGER TOO LARGE]");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // A negative product reports the other direction.
    TEST_PROGRAM_SETUP(R"(print((0 - 9223372036854775807 - 1) * 2);)");
    ASSERT_STREQ(context->output->buffer, "[INTEGER TOO SMALL]");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Negation, where the result is one past the maximum. Both spellings.
    TEST_PROGRAM_SETUP(R"(print(-(0 - 9223372036854775807 - 1));)");
    ASSERT_STREQ(context->output->buffer, "[INTEGER TOO LARGE]");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    TEST_PROGRAM_SETUP(R"(print(0 - (0 - 9223372036854775807 - 1));)");
    ASSERT_STREQ(context->output->buffer, "[INTEGER TOO LARGE]");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // GTA_INTEGER_MIN / -1 is one past the maximum, and raised SIGFPE before
    // it was guarded at all - a killed process from one line of script.
    TEST_PROGRAM_SETUP(R"(print((0 - 9223372036854775807 - 1) / -1);)");
    ASSERT_STREQ(context->output->buffer, "[INTEGER TOO LARGE]");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // But the REMAINDER is representable: x % -1 is 0 for every x, under both
    // the truncating and the flooring convention. Only the hardware objected,
    // because it computes the quotient too. It used to be refused; it has an
    // ordinary answer.
    TEST_PROGRAM_SETUP(R"(print((0 - 9223372036854775807 - 1) % -1); print(","); print(7 % -1);)");
    ASSERT_STREQ(context->output->buffer, "0,0");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Ordinary arithmetic must be untouched, or the guard has simply broken
    // the operators.
    TEST_PROGRAM_SETUP(R"(
      print(2 + 3); print(","); print(10 - 4); print(","); print(6 * 7);
      print(","); print(-5); print(","); print(-(0 - 5));
      print(","); print(10 / 2); print(","); print(10 % 3);
      print(","); print(9223372036854775807); print(","); print(0 - 9223372036854775807 - 1);
    )");
    ASSERT_STREQ(context->output->buffer,
      "5,6,42,-5,5,5,1,9223372036854775807,-9223372036854775808");
    TEST_PROGRAM_TEARDOWN();
  }
}


TEST(Declare, FloatLiteralsDoNotShareASingleton) {
  // Literals are interned in the program's singleton pool, which is keyed by a
  // GTA_UInteger. The float node passed its GTA_Float straight into that
  // parameter, which converts rather than reinterprets, so the key was the
  // truncated integer part and every literal sharing one collapsed onto a
  // single value. `print(1.5); print(1.0);` printed 1.5 twice.
  //
  // Both engines were affected, because the defect is in the shared front end.
  {
    // Same integer part, different values: the case that collided.
    TEST_PROGRAM_SETUP(R"(
      print(1.5); print(","); print(1.0); print(","); print(1.25);
    )");
    ASSERT_STREQ(context->output->buffer, "1.5,1.,1.25");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Integer part 0 collided too, which the 1.x cases alone would not catch.
    TEST_PROGRAM_SETUP(R"(
      print(0.5); print(","); print(0.25);
    )");
    ASSERT_STREQ(context->output->buffer, "0.5,0.25");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Arithmetic on two such literals produced the wrong sum, because the
    // second operand was the first literal's value.
    TEST_PROGRAM_SETUP(R"(
      print(1.5 + 1.0); print(","); print(0.5 + 0.25);
    )");
    ASSERT_STREQ(context->output->buffer, "2.5,0.75");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Interning must still work: one value used twice is still one singleton,
    // and a fix that simply stopped sharing would pass every case above.
    TEST_PROGRAM_SETUP(R"(
      a = 2.75;
      b = 2.75;
      print(a == b);
      print(a); print(","); print(b);
    )");
    // The leading "true" is the `a == b` above. It used to contribute nothing,
    // because printing a boolean rendered as nothing - so the interning check
    // this case exists for was silently asserting only that two prints of 2.75
    // came out as 2.75.
    ASSERT_STREQ(context->output->buffer, "true2.75,2.75");
    TEST_PROGRAM_TEARDOWN();
  }
}


TEST(Declare, LargeFloatRendersWithoutOverflowingItsBuffer) {
  // to_string rendered with %f into a fixed 32-byte allocation. %f writes a
  // large magnitude in full rather than in exponent form, so this overflowed
  // the heap - the first case below aborted inside glibc with a corrupted
  // heap before the buffer was sized from the value.
  {
    TEST_PROGRAM_SETUP(R"(
      a = 1.0;
      for (i = 0; i < 40; i = i + 1) {
        a = a * 10.0;
      }
      print(a);
    )");
    // The exact digits are the double nearest 1e40; what matters is that it
    // renders in full and does not overrun.
    ASSERT_GT(strlen(context->output->buffer), (size_t)32);
    ASSERT_EQ(context->output->buffer[0], '9');
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Far past the old buffer: DBL_MAX needs 316 characters.
    TEST_PROGRAM_SETUP(R"(
      a = 1.0;
      for (i = 0; i < 300; i = i + 1) {
        a = a * 10.0;
      }
      print(a);
    )");
    ASSERT_GT(strlen(context->output->buffer), (size_t)300);
    TEST_PROGRAM_TEARDOWN();
  }
}


TEST(Cast, ToFloat) {
  {
    TEST_REUSABLE_PROGRAM("use a; a as float;");
    {
      // Integer => Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(42.0, ((GTA_Computed_Value_Float *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float => Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
      ASSERT_EQ(3.5, ((GTA_Computed_Value_Float *)context->result)->value);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Boolean => Float.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_bool_true));
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
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_string_42f5));
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
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_42));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
      ASSERT_STREQ("42", ((GTA_Computed_Value_String *)context->result)->value->buffer);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Float => String.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_float_3_5));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
      ASSERT_STREQ("3.5", ((GTA_Computed_Value_String *)context->result)->value->buffer);
      TEST_CONTEXT_TEARDOWN();
    }
    {
      // Boolean => String.
      TEST_CONTEXT_SETUP();
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_bool_true));
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
      ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_string_hello));
      ASSERT_TRUE(gta_program_execute(context));
      ASSERT_TRUE(context->result);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
      ASSERT_STREQ("hello", ((GTA_Computed_Value_String *)context->result)->value->buffer);
      TEST_CONTEXT_TEARDOWN();
    }
    TEST_REUSABLE_PROGRAM_TEARDOWN();
  }
}

TEST(Index, Array) {
  {
    // First element.
    TEST_PROGRAM_SETUP("a = [3, 4.5, true, \"hello\"]; a[0];");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(3, ((GTA_Computed_Value_Integer *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Last element.
    TEST_PROGRAM_SETUP("a = [3, 4.5, true, \"hello\"]; a[3];");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
    ASSERT_STREQ("hello", ((GTA_Computed_Value_String *)context->result)->value->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Out of bounds.
    TEST_PROGRAM_SETUP("a = [3, 4.5, true, \"hello\"]; a[4];");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Negative index from end.
    TEST_PROGRAM_SETUP("a = [3, 4.5, true, \"hello\"]; a[-2];");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(context->result));
    ASSERT_TRUE(((GTA_Computed_Value_Boolean *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Negative index out of bounds.
    TEST_PROGRAM_SETUP("a = [3, 4.5, true, \"hello\"]; a[-5];");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Non-integer index.
    TEST_PROGRAM_SETUP("a = [3, 4.5, true, \"hello\"]; a[3.5];");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ERROR(context->result));
    ASSERT_EQ(context->result, gta_computed_value_error_invalid_index);
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(Index, String) {
  {
    // First character.
    TEST_PROGRAM_SETUP("a = \"hello\"; a[0];");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
    ASSERT_STREQ("h", ((GTA_Computed_Value_String *)context->result)->value->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Last character.
    TEST_PROGRAM_SETUP("a = \"hello\"; a[4];");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
    ASSERT_STREQ("o", ((GTA_Computed_Value_String *)context->result)->value->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Out of bounds.
    TEST_PROGRAM_SETUP("a = \"hello\"; a[5];");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
    ASSERT_STREQ("", ((GTA_Computed_Value_String *)context->result)->value->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Negative index from end.
    TEST_PROGRAM_SETUP("a = \"hello\"; a[-4];");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
    ASSERT_STREQ("e", ((GTA_Computed_Value_String *)context->result)->value->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Negative index out of bounds.
    TEST_PROGRAM_SETUP("a = \"hello\"; a[-6];");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
    ASSERT_STREQ("", ((GTA_Computed_Value_String *)context->result)->value->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Non-integer index.
    TEST_PROGRAM_SETUP("a = \"hello\"; a[3.5];");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ERROR(context->result));
    ASSERT_EQ(context->result, gta_computed_value_error_invalid_index);
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(Index, Map) {
  {
    // Existing key.
    TEST_PROGRAM_SETUP(R"(
      a = {
        foo: 3,
        bar: 4.5,
        baz: true,
        qux: "hello",
      };
      a["bar"];)");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(context->result));
    ASSERT_EQ(4.5, ((GTA_Computed_Value_Float *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Non-existent key.
    TEST_PROGRAM_SETUP(R"(
      a = {
        foo: 3,
        bar: 4.5,
        baz: true,
        qux: "hello",
      };
      a["quux"];)");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Non-string key.
    TEST_PROGRAM_SETUP(R"(
      a = {
        foo: 3,
        bar: 4.5,
        baz: true,
        qux: "hello",
      };
      a[3];)");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ERROR(context->result));
    ASSERT_EQ(context->result, gta_computed_value_error_map_key_not_string);
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(Assignment, ArrayIndex) {
  {
    // Assign to an existing index.
    TEST_PROGRAM_SETUP(R"(a = [3, 4.5, true, "hello"]; a[2] = false; a;)");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ARRAY(context->result));
    GTA_Computed_Value_Array * result = (GTA_Computed_Value_Array *)context->result;
    ASSERT_EQ(4, result->elements->count);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(result->elements->data[0].p));
    ASSERT_EQ(3, ((GTA_Computed_Value_Integer *)result->elements->data[0].p)->value);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(result->elements->data[1].p));
    ASSERT_EQ(4.5, ((GTA_Computed_Value_Float *)result->elements->data[1].p)->value);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(result->elements->data[2].p));
    ASSERT_FALSE(((GTA_Computed_Value_Boolean *)result->elements->data[2].p)->value);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(result->elements->data[3].p));
    ASSERT_STREQ("hello", ((GTA_Computed_Value_String *)result->elements->data[3].p)->value->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Assign to a new index.
    TEST_PROGRAM_SETUP(R"(a = [3, 4.5, true, "hello"]; a[4] = 42; a;)");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ARRAY(context->result));
    GTA_Computed_Value_Array * result = (GTA_Computed_Value_Array *)context->result;
    ASSERT_EQ(5, result->elements->count);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(result->elements->data[0].p));
    ASSERT_EQ(3, ((GTA_Computed_Value_Integer *)result->elements->data[0].p)->value);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(result->elements->data[1].p));
    ASSERT_EQ(4.5, ((GTA_Computed_Value_Float *)result->elements->data[1].p)->value);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(result->elements->data[2].p));
    ASSERT_TRUE(((GTA_Computed_Value_Boolean *)result->elements->data[2].p)->value);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(result->elements->data[3].p));
    ASSERT_STREQ("hello", ((GTA_Computed_Value_String *)result->elements->data[3].p)->value->buffer);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(result->elements->data[4].p));
    ASSERT_EQ(42, ((GTA_Computed_Value_Integer *)result->elements->data[4].p)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Assign to a negative index.
    TEST_PROGRAM_SETUP(R"(a = [3, 4.5, true, "hello"]; a[-2] = 42; a;)");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ARRAY(context->result));
    GTA_Computed_Value_Array * result = (GTA_Computed_Value_Array *)context->result;
    ASSERT_EQ(4, result->elements->count);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(result->elements->data[0].p));
    ASSERT_EQ(3, ((GTA_Computed_Value_Integer *)result->elements->data[0].p)->value);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(result->elements->data[1].p));
    ASSERT_EQ(4.5, ((GTA_Computed_Value_Float *)result->elements->data[1].p)->value);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(result->elements->data[2].p));
    ASSERT_EQ(42, ((GTA_Computed_Value_Integer *)result->elements->data[2].p)->value);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(result->elements->data[3].p));
    ASSERT_STREQ("hello", ((GTA_Computed_Value_String *)result->elements->data[3].p)->value->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Assign to a non-integer index.
    TEST_PROGRAM_SETUP(R"(a = [3, 4.5, true, "hello"]; a[3.5] = 42;)");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ERROR(context->result));
    ASSERT_EQ(context->result, gta_computed_value_error_invalid_index);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Assign to a non-existent index, which results in nulls being added as
    // placeholders.
    TEST_PROGRAM_SETUP(R"(a = [3, 4.5, true, "hello"]; a[6] = 42; a;)");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ARRAY(context->result));
    GTA_Computed_Value_Array * result = (GTA_Computed_Value_Array *)context->result;
    ASSERT_EQ(7, result->elements->count);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(result->elements->data[0].p));
    ASSERT_EQ(3, ((GTA_Computed_Value_Integer *)result->elements->data[0].p)->value);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(result->elements->data[1].p));
    ASSERT_EQ(4.5, ((GTA_Computed_Value_Float *)result->elements->data[1].p)->value);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(result->elements->data[2].p));
    ASSERT_TRUE(((GTA_Computed_Value_Boolean *)result->elements->data[2].p)->value);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(result->elements->data[3].p));
    ASSERT_STREQ("hello", ((GTA_Computed_Value_String *)result->elements->data[3].p)->value->buffer);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(result->elements->data[4].p));
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(result->elements->data[5].p));
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(result->elements->data[6].p));
    ASSERT_EQ(42, ((GTA_Computed_Value_Integer *)result->elements->data[6].p)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Verify that the assignment returns the assigned value.
    TEST_PROGRAM_SETUP(R"(a = [3, 4.5, true, "hello"]; a[2] = 42;)");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
    ASSERT_EQ(42, ((GTA_Computed_Value_Integer *)context->result)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Verify error on invalid (negative, out-of-bounds) index.
    TEST_PROGRAM_SETUP(R"(a = [3, 4.5, true, "hello"]; a[-10] = 42;)");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ERROR(context->result));
    ASSERT_EQ(context->result, gta_computed_value_error_invalid_index);
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(Assignment, MapIndex) {
  {
    // Assign to an existing key.
    TEST_PROGRAM_SETUP(R"(
      a = {
        foo: 3,
        bar: 4.5,
        baz: true,
        qux: "hello",
      };
      a["baz"] = false;
      a;)");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_MAP(context->result));
    GTA_Computed_Value_Map * result = (GTA_Computed_Value_Map *)context->result;
    ASSERT_EQ(4, result->key_hash->entries);
    ASSERT_EQ(4, result->value_hash->entries);
    {
      // foo
      const char * key = "foo";
      GTA_Integer key_hash = gcu_string_hash_64(key, strlen(key));
      GTA_HashX_Value key_result = GTA_HASHX_GET(result->key_hash, key_hash);
      ASSERT_TRUE(key_result.exists);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(GTA_TYPEX_P(key_result.value)));
      ASSERT_STREQ(key, ((GTA_Computed_Value_String *)GTA_TYPEX_P(key_result.value))->value->buffer);
      GTA_HashX_Value value_result = GTA_HASHX_GET(result->value_hash, key_hash);
      ASSERT_TRUE(value_result.exists);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(GTA_TYPEX_P(value_result.value)));
      ASSERT_EQ(3, ((GTA_Computed_Value_Integer *)GTA_TYPEX_P(value_result.value))->value);
    }
    {
      // bar
      const char * key = "bar";
      GTA_Integer key_hash = gcu_string_hash_64(key, strlen(key));
      GTA_HashX_Value key_result = GTA_HASHX_GET(result->key_hash, key_hash);
      ASSERT_TRUE(key_result.exists);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(GTA_TYPEX_P(key_result.value)));
      ASSERT_STREQ(key, ((GTA_Computed_Value_String *)GTA_TYPEX_P(key_result.value))->value->buffer);
      GTA_HashX_Value value_result = GTA_HASHX_GET(result->value_hash, key_hash);
      ASSERT_TRUE(value_result.exists);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(GTA_TYPEX_P(value_result.value)));
      ASSERT_EQ(4.5, ((GTA_Computed_Value_Float *)GTA_TYPEX_P(value_result.value))->value);
    }
    {
      // baz
      const char * key = "baz";
      GTA_Integer key_hash = gcu_string_hash_64(key, strlen(key));
      GTA_HashX_Value key_result = GTA_HASHX_GET(result->key_hash, key_hash);
      ASSERT_TRUE(key_result.exists);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(GTA_TYPEX_P(key_result.value)));
      ASSERT_STREQ(key, ((GTA_Computed_Value_String *)GTA_TYPEX_P(key_result.value))->value->buffer);
      GTA_HashX_Value value_result = GTA_HASHX_GET(result->value_hash, key_hash);
      ASSERT_TRUE(value_result.exists);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(GTA_TYPEX_P(value_result.value)));
      ASSERT_FALSE(((GTA_Computed_Value_Boolean *)GTA_TYPEX_P(value_result.value))->value);
    }
    {
      // qux
      const char * key = "qux";
      GTA_Integer key_hash = gcu_string_hash_64(key, strlen(key));
      GTA_HashX_Value key_result = GTA_HASHX_GET(result->key_hash, key_hash);
      ASSERT_TRUE(key_result.exists);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(GTA_TYPEX_P(key_result.value)));
      ASSERT_STREQ(key, ((GTA_Computed_Value_String *)GTA_TYPEX_P(key_result.value))->value->buffer);
      GTA_HashX_Value value_result = GTA_HASHX_GET(result->value_hash, key_hash);
      ASSERT_TRUE(value_result.exists);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(GTA_TYPEX_P(value_result.value)));
      ASSERT_STREQ("hello", ((GTA_Computed_Value_String *)GTA_TYPEX_P(value_result.value))->value->buffer);
    }
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Assign to a new key.
    TEST_PROGRAM_SETUP(R"(
      a = {
        foo: 3,
      };
      a["bar"] = 42;
      a;)");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_MAP(context->result));
    GTA_Computed_Value_Map * result = (GTA_Computed_Value_Map *)context->result;
    ASSERT_EQ(2, result->key_hash->entries);
    ASSERT_EQ(2, result->value_hash->entries);
    {
      // foo
      const char * key = "foo";
      GTA_Integer key_hash = gcu_string_hash_64(key, strlen(key));
      GTA_HashX_Value key_result = GTA_HASHX_GET(result->key_hash, key_hash);
      ASSERT_TRUE(key_result.exists);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(GTA_TYPEX_P(key_result.value)));
      ASSERT_STREQ(key, ((GTA_Computed_Value_String *)GTA_TYPEX_P(key_result.value))->value->buffer);
      GTA_HashX_Value value_result = GTA_HASHX_GET(result->value_hash, key_hash);
      ASSERT_TRUE(value_result.exists);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(GTA_TYPEX_P(value_result.value)));
      ASSERT_EQ(3, ((GTA_Computed_Value_Integer *)GTA_TYPEX_P(value_result.value))->value);
    }
    {
      // bar
      const char * key = "bar";
      GTA_Integer key_hash = gcu_string_hash_64(key, strlen(key));
      GTA_HashX_Value key_result = GTA_HASHX_GET(result->key_hash, key_hash);
      ASSERT_TRUE(key_result.exists);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(GTA_TYPEX_P(key_result.value)));
      ASSERT_STREQ(key, ((GTA_Computed_Value_String *)GTA_TYPEX_P(key_result.value))->value->buffer);
      GTA_HashX_Value value_result = GTA_HASHX_GET(result->value_hash, key_hash);
      ASSERT_TRUE(value_result.exists);
      ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(GTA_TYPEX_P(value_result.value)));
      ASSERT_EQ(42, ((GTA_Computed_Value_Integer *)GTA_TYPEX_P(value_result.value))->value);
    }
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(Slice, Array) {
  {
    // Slice from start, no skip.
    TEST_PROGRAM_SETUP("[3, 4.5, true, \"hello\"][:2]");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ARRAY(context->result));
    GTA_Computed_Value_Array * result = (GTA_Computed_Value_Array *)context->result;
    ASSERT_EQ(2, result->elements->count);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(result->elements->data[0].p));
    ASSERT_EQ(3, ((GTA_Computed_Value_Integer *)result->elements->data[0].p)->value);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(result->elements->data[1].p));
    ASSERT_EQ(4.5, ((GTA_Computed_Value_Float *)result->elements->data[1].p)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Slice from end with negative index, no end, negative skip.
    TEST_PROGRAM_SETUP("[3, 4.5, true, \"hello\"][-2::-1]");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ARRAY(context->result));
    GTA_Computed_Value_Array * result = (GTA_Computed_Value_Array *)context->result;
    ASSERT_EQ(3, result->elements->count);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_BOOLEAN(result->elements->data[0].p));
    ASSERT_TRUE(((GTA_Computed_Value_Boolean *)result->elements->data[0].p)->value);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_FLOAT(result->elements->data[1].p));
    ASSERT_EQ(4.5, ((GTA_Computed_Value_Float *)result->elements->data[1].p)->value);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(result->elements->data[2].p));
    ASSERT_EQ(3, ((GTA_Computed_Value_Integer *)result->elements->data[2].p)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Slice from negative index (past beginning) to end, positive skip.
    TEST_PROGRAM_SETUP("[0,1,2,3,4,5,6,7,8,9,0][-13::3]");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ARRAY(context->result));
    GTA_Computed_Value_Array * result = (GTA_Computed_Value_Array *)context->result;
    ASSERT_EQ(4, result->elements->count);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(result->elements->data[0].p));
    ASSERT_EQ(1, ((GTA_Computed_Value_Integer *)result->elements->data[0].p)->value);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(result->elements->data[1].p));
    ASSERT_EQ(4, ((GTA_Computed_Value_Integer *)result->elements->data[1].p)->value);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(result->elements->data[2].p));
    ASSERT_EQ(7, ((GTA_Computed_Value_Integer *)result->elements->data[2].p)->value);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(result->elements->data[3].p));
    ASSERT_EQ(0, ((GTA_Computed_Value_Integer *)result->elements->data[3].p)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Slice from index past end to beginning, negative skip.
    TEST_PROGRAM_SETUP("[0,1,2,3,4,5,6,7,8,9,0][19::-5]");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ARRAY(context->result));
    GTA_Computed_Value_Array * result = (GTA_Computed_Value_Array *)context->result;
    ASSERT_EQ(2, result->elements->count);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(result->elements->data[0].p));
    ASSERT_EQ(9, ((GTA_Computed_Value_Integer *)result->elements->data[0].p)->value);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(result->elements->data[1].p));
    ASSERT_EQ(4, ((GTA_Computed_Value_Integer *)result->elements->data[1].p)->value);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Slice with both indices out of bounds, positive skip (negative).
    TEST_PROGRAM_SETUP("[0,1,2,3,4,5,6,7,8,9,0][-20:-15]");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ARRAY(context->result));
    GTA_Computed_Value_Array * result = (GTA_Computed_Value_Array *)context->result;
    ASSERT_EQ(0, result->elements->count);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Slice with both indices out of bounds, negative skip (positive).
    TEST_PROGRAM_SETUP("[0,1,2,3,4,5,6,7,8,9,0][30:20:-1]");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_ARRAY(context->result));
    GTA_Computed_Value_Array * result = (GTA_Computed_Value_Array *)context->result;
    ASSERT_EQ(0, result->elements->count);
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(Slice, String) {
  {
    // Slice from start, no skip.
    TEST_PROGRAM_SETUP(R"("abcdefghijklmnopqrstuvwxyz"[:2])");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
    ASSERT_STREQ("ab", ((GTA_Computed_Value_String *)context->result)->value->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Slice from end with negative index, no end, negative skip.
    TEST_PROGRAM_SETUP(R"("abcdefghijklmnopqrstuvwxyz"[-2::-1])");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
    ASSERT_STREQ("yxwvutsrqponmlkjihgfedcba", ((GTA_Computed_Value_String *)context->result)->value->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Slice from negative index (past beginning) to end, positive skip.
    TEST_PROGRAM_SETUP(R"("abcdefghijklmnopqrstuvwxyz"[-34::3])");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
    ASSERT_STREQ("behknqtwz", ((GTA_Computed_Value_String *)context->result)->value->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Slice from index past end to beginning, negative skip.
    TEST_PROGRAM_SETUP(R"("abcdefghijklmnopqrstuvwxyz"[33::-5])");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
    ASSERT_STREQ("xsnid", ((GTA_Computed_Value_String *)context->result)->value->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Slice with both indices out of bounds, positive skip (negative).
    TEST_PROGRAM_SETUP(R"("abcdefghijklmnopqrstuvwxyz"[-40:-35])");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
    ASSERT_STREQ("", ((GTA_Computed_Value_String *)context->result)->value->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Slice with both indices out of bounds, negative skip (positive).
    TEST_PROGRAM_SETUP(R"("abcdefghijklmnopqrstuvwxyz"[40:30:-1])");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_STRING(context->result));
    ASSERT_STREQ("", ((GTA_Computed_Value_String *)context->result)->value->buffer);
    TEST_PROGRAM_TEARDOWN();
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

TEST(Map, NestedLiteral) {
  // A map literal whose value is another map literal used to segfault under
  // the JIT while it was being built, printed or not.
  //
  // The map compiler saved the key across the value expression with a single
  // push, which left rsp eight bytes out for everything the value compiled
  // to. A literal value survived that; a nested map did not, because
  // building one reaches gcu_string_hash_64(), whose murmur3 state is a
  // stack local the compiler loads with movdqa - an aligned move, which
  // faults on a stack that is not.
  //
  // This file runs twice, once with the JIT disabled and once with the
  // bytecode disabled, so these cover the path that crashed as well as the
  // one that never did.
  {
    TEST_PROGRAM_SETUP("print({a: {b: 1}});");
    ASSERT_TRUE(context->result);
    ASSERT_STREQ("{\"a\": {\"b\": 1}}", context->output->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Nested two deep.
    TEST_PROGRAM_SETUP("print({a: {b: {c: 2}}});");
    ASSERT_TRUE(context->result);
    ASSERT_STREQ("{\"a\": {\"b\": {\"c\": 2}}}", context->output->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Reached through an array, which also crashed.
    TEST_PROGRAM_SETUP("print({a: [1, {b: 2}]});");
    ASSERT_TRUE(context->result);
    ASSERT_STREQ("{\"a\": [1, {\"b\": 2}]}", context->output->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // An empty nested map never crashed - it adds no key and so never
    // hashes one - which is what made the first report look like it was
    // about maps rather than about the stack.
    TEST_PROGRAM_SETUP("print({a: {:}});");
    ASSERT_TRUE(context->result);
    ASSERT_STREQ("{\"a\": {}}", context->output->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // A nested map that is not the first entry, so that the pairs before it
    // have already pushed and popped.
    TEST_PROGRAM_SETUP("m = {a: 1, b: {c: 2}, d: 3}; print(m[\"b\"]);");
    ASSERT_TRUE(context->result);
    ASSERT_STREQ("{\"c\": 2}", context->output->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Two nested maps in one literal.
    TEST_PROGRAM_SETUP("print({a: {b: 1}, c: {d: 2}});");
    ASSERT_TRUE(context->result);
    std::string out{context->output->buffer};
    ASSERT_TRUE(out == "{\"a\": {\"b\": 1}, \"c\": {\"d\": 2}}"
      || out == "{\"c\": {\"d\": 2}, \"a\": {\"b\": 1}}") << out;
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // The value is still reachable afterwards, so the pair really was
    // stored rather than merely surviving construction.
    TEST_PROGRAM_SETUP("m = {a: {b: 42}}; print(m[\"a\"][\"b\"]);");
    ASSERT_TRUE(context->result);
    ASSERT_STREQ("42", context->output->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(Print, Map) {
  {
    // The empty map.
    TEST_PROGRAM_SETUP("print({:});");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
    ASSERT_STREQ("{}", context->output->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // A single entry.  The key is quoted; the value is rendered the way
    // print would show it, as the array does with its elements.
    TEST_PROGRAM_SETUP("print({a: 1});");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
    ASSERT_STREQ("{\"a\": 1}", context->output->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // A nested array renders recursively.
    TEST_PROGRAM_SETUP("print({k: [1, 2]});");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
    ASSERT_STREQ("{\"k\": [1, 2]}", context->output->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Two entries.  Iteration order is the hash table's, so either order is
    // correct; what matters is the separator between them and the absence of
    // one before the closing brace.
    TEST_PROGRAM_SETUP("print({a: 1, b: 2});");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
    std::string out{context->output->buffer};
    ASSERT_TRUE(out == "{\"a\": 1, \"b\": 2}" || out == "{\"b\": 2, \"a\": 1}") << out;
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Enough entries to grow the buffer several times over.  The hand
    // written builder this replaced never wrote the reallocated pointer
    // back, so the first growth left the caller holding a freed block and
    // every entry was written over the one before it; a single short map
    // never grew and so never showed it.
    TEST_PROGRAM_SETUP(
      "print({k00: 0, k01: 1, k02: 2, k03: 3, k04: 4, k05: 5, k06: 6,"
      " k07: 7, k08: 8, k09: 9, k10: 10, k11: 11, k12: 12, k13: 13,"
      " k14: 14, k15: 15, k16: 16, k17: 17, k18: 18, k19: 19});");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
    std::string out{context->output->buffer};

    ASSERT_EQ('{', out.front());
    ASSERT_EQ('}', out.back());
    ASSERT_EQ(std::string::npos, out.find(",}"));
    ASSERT_EQ(19u, std::count(out.begin(), out.end(), ','));
    for (int i = 0; i < 20; ++i) {
      char entry[32];
      snprintf(entry, sizeof(entry), "\"k%02d\": %d", i, i);
      ASSERT_NE(std::string::npos, out.find(entry)) << entry << " missing from " << out;
    }
    TEST_PROGRAM_TEARDOWN();
  }
}

int main(int argc, char **argv) {
  gcu_memory_reset_counts();
  language = gta_language_create();
  size_t alloc_count = gcu_get_alloc_count();
  size_t free_count = gcu_get_free_count();
  assert(language);

  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();

  gcu_memory_reset_counts();
  gta_language_destroy(language);
  assert((alloc_count + gcu_get_alloc_count()) == (free_count + gcu_get_free_count()));

  // ICU cleanup.
  u_cleanup();
  return result;
}

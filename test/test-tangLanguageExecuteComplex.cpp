#include <gtest/gtest.h>
#include <cutil/memory.h>
#include <iostream>
#include <unicode/uclean.h>

#include <tang/tang.h>
#include <tang/macros.h>
#include <tang/computedValue/computedValueAll.h>
#include <tang/program/program.h>
#include <tang/program/bytecode.h>
#include <tang/program/executionContext.h>
#include <tang/program/variable.h>
#include <tang/unicodeString.h>

using namespace std;


GTA_Language * language;


#define TEST_REUSABLE_PROGRAM(code, flags) \
  gcu_memory_reset_counts(); \
  GTA_Program * program = gta_program_create_with_flags(language, code, flags); \
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
  TEST_REUSABLE_PROGRAM(code, GTA_PROGRAM_FLAG_DEFAULT); \
  TEST_CONTEXT_SETUP(); \
  ASSERT_TRUE(gta_program_execute(context)); \
  ASSERT_TRUE(context->result);

#define TEST_PROGRAM_SETUP_NO_RUN(code) \
  TEST_REUSABLE_PROGRAM(code, GTA_PROGRAM_FLAG_DEFAULT); \
  TEST_CONTEXT_SETUP();

#define TEST_PROGRAM_TEARDOWN() \
  TEST_CONTEXT_TEARDOWN(); \
  TEST_REUSABLE_PROGRAM_TEARDOWN();

#define TEST_TEMPLATE_SETUP(code) \
  TEST_REUSABLE_PROGRAM(code, GTA_PROGRAM_FLAG_IS_TEMPLATE); \
  TEST_CONTEXT_SETUP(); \
  ASSERT_TRUE(gta_program_execute(context)); \
  ASSERT_TRUE(context->result);

#define TEST_TEMPLATE_SETUP_NO_RUN(code) \
  TEST_REUSABLE_PROGRAM(code, GTA_PROGRAM_FLAG_IS_TEMPLATE); \
  TEST_CONTEXT_SETUP();


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

TEST(ControlFlow, For) {
  {
    // True condition.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      for (i = 0; i < 3; i = i + 1) {
        print(i);
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
      for (i = 3; i < 3; i = i + 1) {
        print(i);
      }
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start  end");
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(ControlFlow, RangedFor) {
  {
    // Non-empty container.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      for (i : [0, 1, 2]) {
        print(i);
      }
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 012 end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Empty container.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      for (i : []) {
        print(i);
      }
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start  end");
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(ControlFlow, Break) {
  {
    // Break in a while loop.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      i = 0;
      while (i < 4) {
        i = i + 1;
        if (i == 3) {
          break;
        }
        print(i);
      }
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 12 end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Break in a do..while loop.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      i = 0;
      do {
        i = i + 1;
        if (i == 3) {
          break;
        }
        print(i);
      } while (i < 4);
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 12 end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Break in a for loop.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      for (i = 0; i < 4; i = i + 1) {
        if (i == 2) {
          break;
        }
        print(i);
      }
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 01 end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Break outside of a control flow structure.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      break;
      print(" end");
    )");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
    ASSERT_STREQ(context->output->buffer, "start ");
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(ControlFlow, Continue) {
  {
    // Continue in a while loop.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      i = 0;
      while (i < 3) {
        i = i + 1;
        if (i == 2) {
          continue;
        }
        print(i);
      }
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 13 end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Continue in a do..while loop.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      i = 0;
      do {
        i = i + 1;
        if (i == 2) {
          continue;
        }
        print(i);
      } while (i < 3);
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 13 end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Continue in a for loop.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      for (i = 0; i < 3; i = i + 1) {
        if (i == 1) {
          continue;
        }
        print(i);
      }
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 02 end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Continue outside of a control flow structure.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      continue;
      print(" end");
    )");
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
    ASSERT_STREQ(context->output->buffer, "start ");
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(Function, Simple) {
  {
    // Function with no arguments.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      function foo() {
        print("foo");
      }
      foo();
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start foo end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Function with arguments.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      function foo(a, b) {
        print(a + b);
      }
      foo(1, 2);
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 3 end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Function with return value.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      function foo(a, b) {
        return a + b;
      }
      print(foo(1, 2));
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 3 end");
    TEST_PROGRAM_TEARDOWN();
  }
}


TEST(ControlFlowEdgeCases, Break) {
  {
    // Break in global context.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      break;
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start ");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Break in function.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      function foo() {
        print("foo");
        break;
        print("bar");
      }
      foo();
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start foo end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Multiple breaks.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      i = 0;
      while (i < 3) {
        if (i == 2) {
          break;
        }
        print(i);
        i = i + 1;
      }
      i = 0;
      while (i < 3) {
        if (i == 2) {
          break;
        }
        print("a");
        i = i + 1;
        break;
      }
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 01a end");
    TEST_PROGRAM_TEARDOWN();
  }
}


TEST(ControlFlowEdgeCases, Continue) {
  {
    // Continue in global context.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      continue;
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start ");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Continue in function.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      function foo() {
        print("foo");
        continue;
        print("bar");
      }
      foo();
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start foo end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Multiple continues.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      i = 0;
      while (i < 3) {
        i = i + 1;
        if (i == 2) {
          continue;
        }
        print(i);
      }
      i = 0;
      while (i < 3) {
        i = i + 1;
        if (i == 2) {
          continue;
        }
        print("a");
        continue;
      }
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 13aa end");
    TEST_PROGRAM_TEARDOWN();
  }
}


TEST(VariableScope, Global) {
  {
    // Local variable in a function.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      a = 1;
      function foo() {
        a = 2;
        print(a);
      }
      print(a);
      foo();
      print(a);
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 121 end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Global variable in a function, assignment separate.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      a = 1;
      function foo() {
        global a;
        print(a);
        a = 2;
        print(a);
      }
      print(a);
      foo();
      print(a);
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 1122 end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Global variable in a function, assignment combined.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      a = 1;
      function foo() {
        global a = 2;
        print(a);
      }
      print(a);
      foo();
      print(a);
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 122 end");
    TEST_PROGRAM_TEARDOWN();
  }
}


static GTA_Computed_Value * GTA_CALL int_3_callback(GTA_MAYBE_UNUSED(GTA_Computed_Value * bound_object), GTA_MAYBE_UNUSED(GTA_UInteger argc), GTA_MAYBE_UNUSED(GTA_Computed_Value * argv[]), GTA_Execution_Context * context) {
  assert(!bound_object);
  assert(argv);
  return (GTA_Computed_Value *)gta_computed_value_integer_create(3, context);
}

static GTA_Computed_Value * GTA_CALL make_int_3(GTA_Execution_Context * context) {
  return (GTA_Computed_Value *)gta_computed_value_function_native_create(int_3_callback, NULL, context);
}

static GTA_Computed_Value * GTA_CALL add_callback(GTA_MAYBE_UNUSED(GTA_Computed_Value * bound_object), GTA_UInteger argc, GTA_Computed_Value * argv[], GTA_Execution_Context * context) {
  assert(!bound_object);
  assert(argc == 2);
  assert(argv);
  assert(GTA_COMPUTED_VALUE_IS_INTEGER(argv[0]));
  assert(GTA_COMPUTED_VALUE_IS_INTEGER(argv[1]));
  GTA_Computed_Value_Integer * a = (GTA_Computed_Value_Integer *)argv[0];
  GTA_Computed_Value_Integer * b = (GTA_Computed_Value_Integer *)argv[1];
  // Verify that they are given in the expected order.
  // This was a bug in the past, so don't remove this check.
  assert(a->value == 1);
  assert(b->value == 2);
  return (GTA_Computed_Value *)gta_computed_value_integer_create(a->value + b->value, context);
}

static GTA_Computed_Value * GTA_CALL make_add(GTA_Execution_Context * context) {
  return (GTA_Computed_Value *)gta_computed_value_function_native_create(add_callback, NULL, context);
}

static GTA_Computed_Value * GTA_CALL str_len(GTA_Computed_Value * bound_object, GTA_MAYBE_UNUSED(GTA_UInteger argc), GTA_MAYBE_UNUSED(GTA_Computed_Value * argv[]), GTA_Execution_Context * context) {
  assert(bound_object);
  assert(argc == 0);
  assert(GTA_COMPUTED_VALUE_IS_STRING(bound_object));
  GTA_Computed_Value_String * str = (GTA_Computed_Value_String *)bound_object;
  return (GTA_Computed_Value *)gta_computed_value_integer_create(str->value->byte_length, context);
}

static GTA_Computed_Value * GTA_CALL make_str_len(GTA_Execution_Context * context) {
  GTA_Unicode_String * abc = gta_unicode_string_create("abc", 3, GTA_UNICODE_STRING_TYPE_TRUSTED);
  GTA_Computed_Value * str = (GTA_Computed_Value *)gta_computed_value_string_create(abc, true, context);
  return (GTA_Computed_Value *)gta_computed_value_function_native_create(str_len, str, context);
}


TEST(NativeFunction, Library) {
  {
    // Simple function, no arguments
    TEST_PROGRAM_SETUP_NO_RUN(R"(
      use a;
      print("start ");
      print(a());
      print(" end");
    )");
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_int_3));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_STREQ(context->output->buffer, "start 3 end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Simple function, with arguments
    TEST_PROGRAM_SETUP_NO_RUN(R"(
      use a;
      print("start ");
      print(a(1, 2));
      print(" end");
    )");
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_add));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_STREQ(context->output->buffer, "start 3 end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Function with bound object
    TEST_PROGRAM_SETUP_NO_RUN(R"(
      use a;
      print("start ");
      print(a());
      print(" end");
    )");
    ASSERT_TRUE(gta_library_add_library_from_string(context->library, "a", make_str_len));
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_STREQ(context->output->buffer, "start 3 end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Function not found. Does not crash.
    TEST_PROGRAM_SETUP_NO_RUN(R"(
      use a;
      print("start ");
      print(a());
      print(" end");
    )");
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_STREQ(context->output->buffer, "start  end");
    TEST_PROGRAM_TEARDOWN();
  }
}


TEST(Attributes, String) {
  {
    // Length (in graphemes).
    // The long string of hex values is a UTF-8 encoding of the Scottish Flag.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      print("$\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF.".length);
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 3 end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Length (in bytes).
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      print("$\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF.".byte_length);
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 30 end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // HTML encoding.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      print("a&b".html);
      print(" end");
    )");
    GTA_Unicode_Rendered_String rendered = gta_unicode_string_render(context->output);
    ASSERT_TRUE(rendered.buffer);
    ASSERT_STREQ(rendered.buffer, "start a&amp;b end");
    gcu_free(rendered.buffer);
    TEST_PROGRAM_TEARDOWN();
  }
}


TEST(Attributes, Array) {
  {
    // size (empty array).
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      print([].size);
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 0 end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // size (non-empty array).
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      print([1, 2, 3].size);
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 3 end");
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // size (sliced array).
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      a = [1, 2, 3];
      print(a[-2:].size);
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 2 end");
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(Recursion, Fibonacci) {
  {
    // Fibonacci sequence.
    TEST_PROGRAM_SETUP(R"(
      print("start ");
      function fib(n) {
        if (n <= 0) {
          return 0;
        }
        else if (n <= 2) {
          return 1;
        }
        return fib(n - 1) + fib(n - 2);
      }
      print(fib(10));
      print(" end");
    )");
    ASSERT_STREQ(context->output->buffer, "start 55 end");
    TEST_PROGRAM_TEARDOWN();
  }
}

TEST(Execute, Template) {
  {
    // Fibonacci sequence.
    TEST_TEMPLATE_SETUP(R"(<%
      function fib(n) {
        if (n <= 0) {
          return 0;
        }
        else if (n <= 2) {
          return 1;
        }
        return fib(n - 1) + fib(n - 2);
      }
      num = fib(10);
    %>start <%= num %> end)");
    ASSERT_STREQ(context->output->buffer, "start 55 end");
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

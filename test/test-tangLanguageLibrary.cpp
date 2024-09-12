#include <assert.h>
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


TEST(Library, Load) {
  {
    // Math
    TEST_PROGRAM_SETUP("use math; math;");
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_LIBRARY(context->result));
    GTA_Computed_Value_Library * library = (GTA_Computed_Value_Library *)context->result;
    ASSERT_STREQ("math", library->name);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // Random
    TEST_PROGRAM_SETUP("use random; random;");
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_LIBRARY(context->result));
    GTA_Computed_Value_Library * library = (GTA_Computed_Value_Library *)context->result;
    ASSERT_STREQ("random", library->name);
    TEST_PROGRAM_TEARDOWN();
  }
}


TEST(Math, Constants) {
  {
    // Pi
    TEST_PROGRAM_SETUP("use math; print(math.pi);");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
    ASSERT_STREQ("3.141593", context->output->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
}


TEST(Library, UseAs) {
  {
    // math, with library aliased.
    TEST_PROGRAM_SETUP("use math as m; m;");
    ASSERT_TRUE(gta_program_execute(context));
    ASSERT_TRUE(context->result);
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_LIBRARY(context->result));
    GTA_Computed_Value_Library * library = (GTA_Computed_Value_Library *)context->result;
    ASSERT_STREQ("math", library->name);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // math.pi, with library aliased.
    TEST_PROGRAM_SETUP("use math as m; print(m.pi);");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
    ASSERT_STREQ("3.141593", context->output->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
  {
    // math.pi, with pi aliased.
    TEST_PROGRAM_SETUP("use math.pi as pi; print(pi);");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_NULL(context->result));
    ASSERT_STREQ("3.141593", context->output->buffer);
    TEST_PROGRAM_TEARDOWN();
  }
}


TEST(Random, Random) {
  {
    // Random
    TEST_PROGRAM_SETUP("use random; random.global.next_int;");
    ASSERT_TRUE(GTA_COMPUTED_VALUE_IS_INTEGER(context->result));
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

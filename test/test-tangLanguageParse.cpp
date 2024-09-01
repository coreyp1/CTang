
#include <gtest/gtest.h>
#include <iostream>
#include <cutil/memory.h>

#include <tang/macros.h>
#include <tang/ast/astNodeAll.h>
#include <tang/tangLanguage.h>

using namespace std;

TEST(TangLanguage, test1) {
  {
    // Parse an empty string.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("");
    ASSERT_EQ(ast, nullptr);
    ASSERT_EQ(0, gta_tang_node_count(ast));
    if (ast) gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Parse a valid script.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(R"(
      use math.floor as floor;
      use y;
      use ignore_me;
      z = 3;
      function f(x) {
        function fInner(floor) {
          global z;
          return floor < z ? floor : z;
        }
        return fInner(x);
      }
      function g(y) {
        function fInner(foo) {
            return floor(f(foo));
        }
        return fInner(y);
      }
      y = 42;
      print(g(y));
    )");
    ASSERT_NE(ast, nullptr);
    if (ast) gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}

TEST(Binary, Integers) {
  {
    // Int + Int
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("1 + 2");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(3, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Int / 0
    // Should not simplify.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("1 / 0");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(3, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Multiple statements.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("[1, 2 + 3 + 4, 5 * 6 * 7]");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(12, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(4, gta_tang_node_count(ast));
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}

TEST(Binary, Floats) {
  {
    // Float + Float
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("1.1 + 2.2");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(3, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Float / 0
    // Should not simplify.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("1.1 / 0");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(3, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Multiple statements.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("[1.1, 2.2 + 3.3 + 4.4, 5.5 * 6.6 * 7.7]");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(12, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(4, gta_tang_node_count(ast));
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}

TEST(Binary, Boolean) {
  {
    // Convert to boolean (true).
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("1 + 2. == 3");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(5, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_BOOLEAN(ast));
    ASSERT_TRUE(((GTA_Ast_Node_Boolean *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Convert to boolean (false).
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("1 + 2. < 3");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(5, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_BOOLEAN(ast));
    ASSERT_FALSE(((GTA_Ast_Node_Boolean *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Boolean (true).
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("true && false || 3");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(5, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(((GTA_Ast_Node_Boolean *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Boolean (false).
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("true && false && 3");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(5, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_FALSE(((GTA_Ast_Node_Boolean *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}

TEST(Binary, String) {
  {
    // String concatenation.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(R"("a" + "b" + "c")");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(5, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_STRING(ast));
    ASSERT_STREQ("abc", ((GTA_Ast_Node_String *)ast)->string->buffer);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // String comparison of different values.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(R"("a" == "b")");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(3, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_BOOLEAN(ast));
    ASSERT_FALSE(((GTA_Ast_Node_Boolean *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // String comparison of same values.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(R"("a" == "a")");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(3, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_BOOLEAN(ast));
    ASSERT_TRUE(((GTA_Ast_Node_Boolean *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}

TEST(Unary, Negative) {
  {
    // Negative of an integer.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("-3");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_INTEGER(ast));
    ASSERT_EQ(-3, ((GTA_Ast_Node_Integer *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Negative of a float.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("-3.3");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_FLOAT(ast));
    ASSERT_EQ(-3.3, ((GTA_Ast_Node_Float *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}

TEST(Unary, Not) {
  {
    // Not of a boolean.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("!true");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_BOOLEAN(ast));
    ASSERT_FALSE(((GTA_Ast_Node_Boolean *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Double not of a boolean.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("!!true");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(3, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_BOOLEAN(ast));
    ASSERT_TRUE(((GTA_Ast_Node_Boolean *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Not of a non-zero integer.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("!3");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_BOOLEAN(ast));
    ASSERT_FALSE(((GTA_Ast_Node_Boolean *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Not of a zero integer.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("!0");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_BOOLEAN(ast));
    ASSERT_TRUE(((GTA_Ast_Node_Boolean *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Not of a non-zero float.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("!3.3");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_BOOLEAN(ast));
    ASSERT_FALSE(((GTA_Ast_Node_Boolean *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Not of a zero float.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("!0.0");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_BOOLEAN(ast));
    ASSERT_TRUE(((GTA_Ast_Node_Boolean *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Not of an empty string.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(R"(! "")");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_BOOLEAN(ast));
    ASSERT_TRUE(((GTA_Ast_Node_Boolean *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Not of a non-empty string.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(R"(! "a")");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_BOOLEAN(ast));
    ASSERT_FALSE(((GTA_Ast_Node_Boolean *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}

TEST(Variable, SimpleReplacement) {
  {
    // Single Variable.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("x = 3; y = x + x * x;");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(11, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(7, gta_tang_node_count(ast));
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}

TEST(Variable, DetectEmbeddedConditionalAssignment) {
  {
    // Assignment inside a ternary expression should invalidate the cached
    // value of the assigned variable.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(R"(
      x = 3;
      y = z ? x = 2 : 1;
      y = x + x * x;
    )");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(19, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(19, gta_tang_node_count(ast));
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Assignment inside an if() statement should invalidate the cached
    // value of the assigned variable.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(R"(
      x = 3;
      if (z) {
        x = 2;
      }
      y = x + x * x;
    )");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(17, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(17, gta_tang_node_count(ast));
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Assignment inside an else of an if() statement should invalidate the
    // cached value of the assigned variable.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(R"(
      x = 3;
      if (z) {
      } else {
        x = 2;
      }
      y = x + x * x;
    )");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(19, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(19, gta_tang_node_count(ast));
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}

TEST(Variable, DetectEmbeddedLoopAssignments) {
  {
    // Assignment inside a while() statement should invalidate the cached
    // value of the assigned variable.
    // 'x' should be invalidated.
    // 'w' should not be invalidated.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(R"(
      x = 3;
      w = 1;
      while (z) {
        x = x + 1;
      }
      y = x + x * x + (w + w);
    )");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(26, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(24, gta_tang_node_count(ast));
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Assignment inside a do-while() statement should invalidate the cached
    // value of the assigned variable.
    // 'x' should be invalidated.
    // 'w' should not be invalidated.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(R"(
      x = 3;
      w = 1;
      do {
        x = x + 1;
      } while (z);
      y = x + x * x + (w + w);
    )");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(26, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(24, gta_tang_node_count(ast));
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Assignment inside a for() statement should invalidate the cached
    // value of the assigned variable.
    // 'x' should be invalidated.
    // 'i' should be invalidated.
    // 'w' should not be invalidated.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(R"(
      x = 3;
      w = 1;
      for (i = 0; i < 10; i = i + w) {
        x = x + 1;
      }
      y = x + x * x + (w + w) + (i + i);
    )");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(40, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(38, gta_tang_node_count(ast));
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Assignment inside a ranged-for statement should invalidate the cached
    // value of the assigned variable.
    // 'x' should be invalidated.
    // 'i' should be invalidated.
    // 'w' should not be invalidated.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(R"(
      x = 3;
      for (i : [1, 2, w = 3]) {
        i = w + 1;
        x = x + 1;
      }
      y = x + x * x + (w + w) + (i + i);
    )");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(39, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(35, gta_tang_node_count(ast));
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}

TEST(Function, NewVariableMap) {
  {
    // Function with a new variable map.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(R"(
      x = 3;
      function f(z) {
        y = x + 1;
        y = y + z;
        return y;
      }
      z = f(3);
    )");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(24, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(24, gta_tang_node_count(ast));
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}

TEST(Cast, FromInteger) {
  {
    // Cast from integer to integer.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("3 as int");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_INTEGER(ast));
    ASSERT_EQ(3, ((GTA_Ast_Node_Integer *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Cast from integer to float.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("3 as float");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_FLOAT(ast));
    ASSERT_EQ(3.0, ((GTA_Ast_Node_Float *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Cast from integer to boolean (true).
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("3 as bool");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_BOOLEAN(ast));
    ASSERT_TRUE(((GTA_Ast_Node_Boolean *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Cast from integer to boolean (false).
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("0 as bool");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_BOOLEAN(ast));
    ASSERT_FALSE(((GTA_Ast_Node_Boolean *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Cast from integer to string.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("3 as string");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_STRING(ast));
    ASSERT_STREQ("3", ((GTA_Ast_Node_String *)ast)->string->buffer);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}

TEST(Cast, FromFloat) {
  {
    // Cast from float to integer.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("3.3 as int");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_INTEGER(ast));
    ASSERT_EQ(3, ((GTA_Ast_Node_Integer *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Cast from float to float.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("3.3 as float");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_FLOAT(ast));
    ASSERT_EQ(3.3, ((GTA_Ast_Node_Float *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Cast from float to boolean (true).
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("3.3 as bool");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_BOOLEAN(ast));
    ASSERT_TRUE(((GTA_Ast_Node_Boolean *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Cast from float to boolean (false).
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("0.0 as bool");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_BOOLEAN(ast));
    ASSERT_FALSE(((GTA_Ast_Node_Boolean *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Cast from float to string.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("3.3 as string");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_STRING(ast));
    ASSERT_STREQ("3.3", ((GTA_Ast_Node_String *)ast)->string->buffer);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}

TEST(Cast, FromBoolean) {
  {
    // Cast from boolean (true) to integer.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("true as int");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_INTEGER(ast));
    ASSERT_EQ(1, ((GTA_Ast_Node_Integer *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Cast from boolean (false) to integer.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("false as int");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_INTEGER(ast));
    ASSERT_EQ(0, ((GTA_Ast_Node_Integer *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Cast from boolean (true) to float.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("true as float");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_FLOAT(ast));
    ASSERT_EQ(1.0, ((GTA_Ast_Node_Float *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Cast from boolean (false) to float.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("false as float");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_FLOAT(ast));
    ASSERT_EQ(0.0, ((GTA_Ast_Node_Float *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Cast from boolean (true) to boolean.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("true as bool");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_BOOLEAN(ast));
    ASSERT_TRUE(((GTA_Ast_Node_Boolean *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Cast from boolean (false) to boolean.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("false as bool");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_BOOLEAN(ast));
    ASSERT_FALSE(((GTA_Ast_Node_Boolean *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Cast from boolean (true) to string.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("true as string");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_STRING(ast));
    ASSERT_STREQ("true", ((GTA_Ast_Node_String *)ast)->string->buffer);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Cast from boolean (false) to string.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("false as string");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_STRING(ast));
    ASSERT_STREQ("false", ((GTA_Ast_Node_String *)ast)->string->buffer);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}

TEST(Cast, FromString) {
  {
    // Cast from string to integer.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(R"("3" as int)");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_INTEGER(ast));
    ASSERT_EQ(3, ((GTA_Ast_Node_Integer *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Cast from string to float.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(R"("3.3" as float)");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_FLOAT(ast));
    ASSERT_EQ(3.3, ((GTA_Ast_Node_Float *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Cast from string to boolean (true).
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(R"(":-D" as bool)");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_BOOLEAN(ast));
    ASSERT_TRUE(((GTA_Ast_Node_Boolean *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Cast from string to boolean (false).
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(R"("" as bool)");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_BOOLEAN(ast));
    ASSERT_FALSE(((GTA_Ast_Node_Boolean *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Cast from string to string.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(R"("3" as string)");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_STRING(ast));
    ASSERT_STREQ("3", ((GTA_Ast_Node_String *)ast)->string->buffer);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}

TEST(String, EscapedCharacters) {
  {
    // Multi-codepoint graphemes.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(R"("$\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF.")");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(1, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_STRING(ast));
    ASSERT_STREQ("$\xF0\x9F\x8F\xB4\xF3\xA0\x81\xA7\xF3\xA0\x81\xA2\xF3\xA0\x81\xB3\xF3\xA0\x81\xA3\xF3\xA0\x81\xB4\xF3\xA0\x81\xBF.", ((GTA_Ast_Node_String *)ast)->string->buffer);
    ASSERT_EQ(3, ((GTA_Ast_Node_String *)ast)->string->grapheme_length);
    ASSERT_EQ(30, ((GTA_Ast_Node_String *)ast)->string->byte_length);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}

// Other optimizations:
// - Remove unused variables.

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

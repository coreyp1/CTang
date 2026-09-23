
#include <gtest/gtest.h>
#include <iostream>
#include <ghoti.io/cutil/memory.h>

#include <ghoti.io/tang/macros.h>
#include <ghoti.io/tang/ast/astNodeAll.h>
#include <ghoti.io/tang/tangLanguage.h>

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

TEST(Parse, FailingRuleFreesWhatItWasHanded) {
  // A rule action that bails is the last owner of everything it was handed:
  // bison has already popped the RHS into $1..$n, so the %destructor will not
  // run for those. VERIFY1..4 therefore have to destroy what they discard, and
  // every owning token in a rule's RHS has to be passed to them - ten arms
  // named an expression but not the IDENTIFIER beside it.
  //
  // Each case below leaked before that change. They are short because the
  // fuzzer minimised them; each is the smallest input reaching its arm.
  for (const char * src : {
      "s=\"\xc8\"",     // assignment: VERIFY2 discarded $1 and $3 without freeing
      "\"\xa0\";r",     // trailing identifier, never consumed by a rule
      "\"\xff\".g",     // member access: the IDENTIFIER was not passed to VERIFY
      "f(\"\xff\")",    // call argument list
      "[\"\xff\"]",     // array literal
      "{a:\"\xff\"}",   // map literal, key and value
      "for(i:\"\xff\"){}",
      "function f(\"\xff\"){}",
      // Scanner side, same shape: an error return that skipped CLEANUP_BUFFER.
      // An octal escape above 255, with characters already accumulated - the
      // shortest case, "\\421 with nothing before it, does not leak, which is
      // how this survived a first look.
      "\"1\\421",
      "\"abc\\777def\"",
      }) {
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(src);
    if (ast) {
      gta_ast_node_destroy(ast);
    }
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count()) << "leaked on: " << src;
  }
}


TEST(Parse, ConstantFoldingDoesNotOverflow) {
  // Folding must not compute what the expression could not. Signed overflow is
  // undefined behaviour, so these were UB in the compiler rather than a wrong
  // answer in the program, and the folded constant was whatever the optimiser
  // decided. The folder now declines, exactly as it already did for division
  // by zero, and leaves the operation to run time.
  //
  // UBSan is what catches the overflow itself; `make test-asan` runs this file.
  // These assertions check the part visible in any build: it still parses, it
  // does not crash, and it balances.
  for (const char * src : {
      "9223372036854775807 * 9223372036854775807;",
      "9223372036854775807 + 1;",
      "0 - 9223372036854775807 - 2;",
      "(0 - 9223372036854775807 - 1) / -1;",   // quotient is one past the max
      "(0 - 9223372036854775807 - 1) % -1;",
      "10 / 0;",
      "7 % 0;",
      }) {
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(src);
    ASSERT_NE(ast, nullptr) << "failed to parse: " << src;
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count()) << "leaked on: " << src;
  }
  {
    // And folding that cannot overflow must still happen, or the guard has
    // simply turned constant folding off.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("2 * 3");
    ASSERT_NE(ast, nullptr);
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_TRUE(GTA_AST_IS_INTEGER(ast));
    ASSERT_EQ(6, ((GTA_Ast_Node_Integer *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}


TEST(Parse, InvalidUtf8InATemplateDoesNotLeaveAFreedPointer) {
  // One byte. `\xc7` as a whole template was a heap-use-after-free.
  //
  // The TEMPLATESTRING arm calls gta_unicode_string_create_and_adopt, which
  // fails on invalid UTF-8 - ordinary input, not an allocation failure. It
  // freed the token's buffer and broke without clearing $$. Bison initialises
  // $$ to $1, and $1 here is the token, whose first union member is the .str
  // just freed, so the cleanup path then ran gta_ast_node_destroy on a freed
  // buffer.
  //
  // This is the identical defect that was fixed in the STRING arm, in the
  // sibling arm the parse-only fuzzer cannot reach. The template fuzzer found
  // it in 16 executions the first time it was ever run.
  //
  // The same arm also tested `if (!$$)` where it meant `if (!template_string)`,
  // so that guard could never fire.
  for (const char * src : {
      "\xc7",
      "<%= \"\xc7" "a\" %>",   // split: \xc7a would be one over-long hex escape
      "\xdb\xdf plain text \xc7",
      }) {
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_template(src);
    // Rejected or accepted are both fine; not crashing and not leaking is the
    // property. ASan is what catches the use-after-free half; `make test-asan`
    // runs this file.
    if (ast) {
      gta_ast_node_destroy(ast);
    }
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count()) << "leaked on: " << src;
  }
  {
    // A valid template must still parse, or the guard has broken templates.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_template("a <%= \"b\" %> c");
    ASSERT_NE(ast, nullptr);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}


TEST(Parse, QuickPrintFreesTheExpressionItWasHanded) {
  // `\x87<%=t%>` leaked the parsed expression - seven bytes.
  //
  // The QUICKPRINTBEGINANDSTRING arm is handed two things it owns: the
  // token's string and $2, the expression already parsed between the
  // delimiters. Its early exits freed only the string. That matters because
  // the first of them is reached by ordinary input rather than by allocation
  // failure: the leading template text is part of the same token, and
  // gta_unicode_string_create_and_adopt rejects invalid UTF-8.
  //
  // The rule is the one this grammar keeps running into - by the time an
  // action runs, the RHS has been popped into $1..$n, so a %destructor will
  // not run for any of it, and an action that bails is the last owner of
  // everything it was handed.
  for (const char * src : {
      "\x87<%=t%>",
      "\x87<%= \"a\" + b %>",
      "\xdb<%=t%> tail <%=u%>",
      }) {
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_template(src);
    if (ast) {
      gta_ast_node_destroy(ast);
    }
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count()) << "leaked on: " << src;
  }
  {
    // The same shape with valid UTF-8 must still parse, or the guard has
    // broken quick print.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_template("hi <%= t %>");
    ASSERT_NE(ast, nullptr);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}


TEST(Parse, StringToIntegerIsNotFolded) {
  // The folder used to convert a constant string to an integer by parsing it
  // as a FLOAT and truncating. That disagreed with the runtime conversion,
  // which parses an integer as the language reference specifies - `"1e3" as
  // int` folded to 1000 and executed as 1 - and the truncation was the same
  // out-of-range undefined behaviour as the float cast, for any string naming
  // a number too large to fit.
  //
  // It now declines, so run time is the only place the rule lives. Nothing in
  // the library calls gta_tang_simplify today (the call in gta_tang_primary_parse
  // is commented out), so the divergence was latent rather than visible - but
  // it is a public API, and the fuzzer reaches it.
  for (const char * src : {
      "\"99999999999999999999999\" as int;",
      "\"-99999999999999999999999\" as int;",
      "\"1e3\" as int;",
      }) {
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(src);
    ASSERT_NE(ast, nullptr) << "failed to parse: " << src;
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr) << src;
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count()) << "leaked on: " << src;
  }
  {
    // The cast survives simplification rather than becoming an integer.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("\"42\" as int");
    ASSERT_NE(ast, nullptr);
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_TRUE(GTA_AST_IS_CAST(ast));
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}


TEST(Parse, OutOfRangeCastDoesNotFold) {
  // Converting a float that does not fit into an integer is undefined
  // behaviour, and the folder did it at compile time. That is the site the
  // fuzzer reached, because it parses and simplifies without executing, and
  // under -fno-sanitize-recover it aborts - which is how a fuzz run ended with
  // no artifact at all.
  //
  // The folder now declines, leaving the cast node for run time to answer with
  // [INTEGER TOO LARGE], [INTEGER TOO SMALL] or [NOT A NUMBER]. Declining rather than folding
  // to a marker is deliberate: there is no AST node that could carry one, and
  // it means the two engines and the folder cannot drift apart, because only
  // one of them decides.
  for (const char * src : {
      "77777777777777777777777.9 as int;",
      "(0.0 - 77777777777777777777777.9) as int;",
      "9223372036854775807.0 as int;",           // rounds up to 2^63, does not fit
      }) {
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(src);
    ASSERT_NE(ast, nullptr) << "failed to parse: " << src;
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr) << src;
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count()) << "leaked on: " << src;
  }
  {
    // The cast survives simplification rather than becoming an integer, which
    // is what leaves the answer to run time.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("77777777777777777777777.9 as int");
    ASSERT_NE(ast, nullptr);
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_TRUE(GTA_AST_IS_CAST(ast));
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // And a cast that does fit must still fold, or the guard has simply turned
    // this fold off.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("3.9 as int");
    ASSERT_NE(ast, nullptr);
    ast = gta_tang_simplify(ast);
    ASSERT_NE(ast, nullptr);
    ASSERT_TRUE(GTA_AST_IS_INTEGER(ast));
    ASSERT_EQ(3, ((GTA_Ast_Node_Integer *)ast)->value);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}


TEST(Parse, InvalidUtf8InAStringLiteral) {
  // A string literal whose bytes are not valid UTF-8 makes
  // gta_unicode_string_create_and_adopt fail. That arm freed the token's
  // buffer and broke without assigning $$, so the value union still held the
  // token - whose first member is the pointer just freed - and the start rule
  // took it as the AST. Walking it was a use-after-free, from three bytes of
  // input.
  //
  // Three separate defects had to line up: the arm not clearing $$, the start
  // rule not running VERIFY1, and ~86 sites assigning `parseError` (the
  // parameter) instead of `*parseError`, so nothing downstream ever learned an
  // error had occurred.
  //
  // These assertions catch the leak half in any build. The use-after-free half
  // needs `make test-asan`, which runs this file under ASan.
  {
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("\"\xff\"");
    // Rejection is a parse-error node, not null. Null means "there was
    // nothing to parse" - an empty source is valid - and collapsing the two
    // is what let a syntax error compile into an empty program.
    ASSERT_NE(ast, nullptr);
    ASSERT_TRUE(GTA_AST_IS_PARSE_ERROR(ast));
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // With a trailing token, so error recovery discards a symbol and runs its
    // %destructor. That destructor was not null-safe, and once the arm above
    // started clearing $$ correctly it received null and aborted on an assert.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("\"\xff\"<");
    ASSERT_NE(ast, nullptr);
    ASSERT_TRUE(GTA_AST_IS_PARSE_ERROR(ast));
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // A lone continuation byte, and a lead byte with no continuation.
    for (const char * src : {"\"\x80\"", "\"\xc3\"", "\"\xf0\x9f\""}) {
      gcu_memory_reset_counts();
      GTA_Ast_Node * ast = gta_tang_parse_script(src);
      ASSERT_NE(ast, nullptr) << src;
      ASSERT_TRUE(GTA_AST_IS_PARSE_ERROR(ast)) << src;
      gta_ast_node_destroy(ast);
      ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count()) << src;
    }
  }
  {
    // Valid UTF-8 must still parse, or the fix is just "reject everything".
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("\"caf\xc3\xa9\";");
    ASSERT_NE(ast, nullptr);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // And a syntax error must still yield a parse-error node rather than null:
    // gta_program_create distinguishes the two, so returning null here would
    // make an invalid program look like an empty one.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("invalid syntax :(");
    ASSERT_NE(ast, nullptr);
    gta_ast_node_destroy(ast);
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
}


TEST(Parse, UnterminatedStringDoesNotLeak) {
  // The scanner accumulates a string literal into a heap buffer. Every error
  // return in tangScanner.l frees it with CLEANUP_BUFFER except the one for
  // end-of-input inside a literal, which simply returned STRINGERROR - so
  // input ending mid-string leaked whatever had been accumulated. Nothing
  // downstream ever sees that buffer, so it could not be freed later.
  //
  // Found by the parse fuzzer within 30 executions; this is the minimised
  // case. The empty literal does not leak, because nothing was accumulated,
  // so a test using only `"` would have passed against the bug.
  {
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("x = \"abc");
    if (ast) {
      gta_ast_node_destroy(ast);
    }
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // Longer, to be sure this is the accumulated literal and not a fixed
    // allocation that happens to be the same size.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("x = \"abcdefghijklmnopqrstuvwxyz0123456789");
    if (ast) {
      gta_ast_node_destroy(ast);
    }
    ASSERT_EQ(gcu_get_alloc_count(), gcu_get_free_count());
  }
  {
    // The terminated form must still parse and still balance.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script("x = \"abc\";");
    ASSERT_NE(ast, nullptr);
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
    // Cast from string to integer is deliberately NOT folded any more, so the
    // cast node survives. It used to fold by parsing the string as a float and
    // truncating, which disagreed with the runtime conversion and was
    // undefined behaviour for a string naming a number too large to fit. The
    // conversion is not reimplemented here: duplicating a parsing rule is what
    // let the two drift apart, and run time is the only thing that can produce
    // [INTEGER TOO LARGE] anyway. See Parse.StringToIntegerIsNotFolded.
    gcu_memory_reset_counts();
    GTA_Ast_Node * ast = gta_tang_parse_script(R"("3" as int)");
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ast = gta_tang_simplify(ast);
    ASSERT_EQ(2, gta_tang_node_count(ast));
    ASSERT_TRUE(GTA_AST_IS_CAST(ast));
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

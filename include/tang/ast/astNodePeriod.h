/**
 * @file
 */

#ifndef GTA_AST_NODE_PERIOD_H
#define GTA_AST_NODE_PERIOD_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <tang/ast/astNode.h>

/**
 * The vtable for the GTA_Ast_Node_Period class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_period_vtable;

/**
 * The GTA_Ast_Node_Period class.
 */
struct GTA_Ast_Node_Period {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The left-hand side of the period operation.
   */
  GTA_Ast_Node * lhs;
  /**
   * The right-hand side of the period operation.
   */
  const char * rhs;
};

/**
 * Creates a new GTA_Ast_Node_Period object.
 *
 * The `rhs` string is adopted by the new object and should not be freed by the
 * caller.
 *
 * @param lhs The left-hand side of the period operation.
 * @param rhs The right-hand side of the period operation.
 * @param location The location of the period operation in the source code.
 * @return The new GTA_Ast_Node_Period object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_Period * gta_ast_node_period_create(GTA_Ast_Node * lhs, const char * rhs, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Period object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Period object to destroy.
 */
void gta_ast_node_period_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Period object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Period object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_period_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Period object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Period object to simplify.
 * @param variable_map The variable map to use when simplifying the node.
 * @return The simplified node or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_period_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_Period object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Period object to walk.
 * @param callback The callback function to call for each node in the tree.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_period_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

/**
 * Perform pre-compilation analysis on the AST node.
 *
 * This step includes allocating constants, identifying libraries and variables
 * (global and local), and creating namespace scopes for functions.
 *
 * This function should not be called directly. Use gta_ast_node_analyze()
 * instead.
 *
 * @param self The node to analyze.
 * @param program The program that the node is part of.
 * @return NULL on success, otherwise return a parse error.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_period_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope);

/**
 * Compile the AST node to binary for x86_64.
 *
 * This function should not be called directly. Use gta_ast_node_compile_to_binary()
 * instead.
 *
 * @see gta_ast_node_compile_to_binary__x86_64
 *
 * @param self The node to compile.
 * @param context Contextual information for the compile process.
 * @return True on success, false on failure.
 */
bool gta_ast_node_period_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context);

/**
 * Compiles the AST node to bytecode.
 *
 * This function should not be called directly. Use gta_ast_node_compile_to_bytecode()
 * instead.
 *
 * @see gta_ast_node_compile_to_bytecode
 *
 * @param self The node to compile.
 * @param context The compiler state to use for compilation.
 */
bool gta_ast_node_period_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_PERIOD_H
/**
 * @file
 */

#ifndef GTA_AST_NODE_ASSIGN_H
#define GTA_AST_NODE_ASSIGN_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <tang/ast/astNode.h>

/**
 * The vtable for the GTA_Ast_Node_Assign class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_assign_vtable;

/**
 * The GTA_Ast_Node_Assign class.
 */
typedef struct GTA_Ast_Node_Assign {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The left-hand side of the assignment.
   */
  GTA_Ast_Node * lhs;
  /**
   * The right-hand side of the assignment.
   */
  GTA_Ast_Node * rhs;
} GTA_Ast_Node_Assign;

/**
 * Creates a new GTA_Ast_Node_Assign object.
 *
 * @param lhs The left-hand side of the assignment.
 * @param rhs The right-hand side of the assignment.
 * @param location The location of the assignment in the source code.
 * @return The new GTA_Ast_Node_Assign object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_Assign * gta_ast_node_assign_create(GTA_Ast_Node * lhs, GTA_Ast_Node * rhs, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Assign object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Assign object to destroy.
 */
void gta_ast_node_assign_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Assign object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Assign object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_assign_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Assign object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Assign object to simplify.
 * @param variable_map The variable map to use for simplification.
 * @return The simplified GTA_Ast_Node_Assign object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_assign_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Perform pre-compilation analysis on the AST node.
 *
 * This step includes allocating constants, identifying libraries and variables
 * (global and local), and creating namespace scopes for functions.
 *
 * This function serves as a general dispatch function, and should be used in
 * preference to calling the vtable's analyze function directly.
 *
 * @see gta_ast_node_analyze()
 *
 * @param self The node to analyze.
 * @param program The program that the node is part of.
 * @param scope The current variable scope.
 * @return NULL on success, otherwise return a parse error.
 */
GTA_Ast_Node * gta_ast_node_assign_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope);

/**
 * Walks a GTA_Ast_Node_Assign object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Assign object to walk.
 * @param callback The callback to call for each node.
 * @param data The user-defined data to pass to the callback.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_assign_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

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
bool gta_ast_node_assign_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context);

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
bool gta_ast_node_assign_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_ASSIGN_H
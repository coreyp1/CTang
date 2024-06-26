/**
 * @file
 */

#ifndef GTA_AST_NODE_SLICE_H
#define GTA_AST_NODE_SLICE_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <tang/ast/astNode.h>

/**
 * The vtable for the GTA_Ast_Node_Slice class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_slice_vtable;

/**
 * The GTA_Ast_Node_Slice class.
 */
typedef struct GTA_Ast_Node_Slice {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The left-hand side of the slice operation.
   */
  GTA_Ast_Node * lhs;
  /**
   * The start of the slice operation.
   */
  GTA_Ast_Node * start;
  /**
   * The end of the slice operation.
   */
  GTA_Ast_Node * end;
  /**
   * The skip of the slice operation.
   */
  GTA_Ast_Node * skip;
} GTA_Ast_Node_Slice;

/**
 * Creates a new GTA_Ast_Node_Slice object.
 *
 * @param lhs The left-hand side of the slice operation.
 * @param start The start of the slice operation.
 * @param end The end of the slice operation.
 * @param skip The skip of the slice operation.
 * @param location The location of the slice operation in the source code.
 * @return The new GTA_Ast_Node_Slice object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_Slice * gta_ast_node_slice_create(GTA_Ast_Node * lhs, GTA_Ast_Node * start, GTA_Ast_Node * end, GTA_Ast_Node * skip, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Slice object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Slice object to destroy.
 */
void gta_ast_node_slice_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Slice object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Slice object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_slice_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Slice object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Slice object to simplify.
 * @param variable_map The variable map to use for simplification.
 * @return The simplified GTA_Ast_Node_Slice object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_slice_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_Slice object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Slice object to walk.
 * @param callback The callback to call for each node in the tree.
 * @param data The user-defined data to pass to the callback.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_slice_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

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
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_binary_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_SLICE_H
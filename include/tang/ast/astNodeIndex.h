/**
 * @file
 */

#ifndef GTA_AST_NODE_INDEX_H
#define GTA_AST_NODE_INDEX_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <tang/ast/astNode.h>

/**
 * The vtable for the GTA_Ast_Node_Index class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_index_vtable;

/**
 * The GTA_Ast_Node_Index class.
 */
typedef struct GTA_Ast_Node_Index {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The left-hand side of the index operation.
   */
  GTA_Ast_Node * lhs;
  /**
   * The right-hand side of the index operation.
   */
  GTA_Ast_Node * rhs;
} GTA_Ast_Node_Index;

/**
 * Creates a new GTA_Ast_Node_Index object.
 *
 * @param lhs The left-hand side of the index operation.
 * @param rhs The right-hand side of the index operation.
 * @param location The location of the index operation in the source code.
 * @return The new GTA_Ast_Node_Index object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_Index * gta_ast_node_index_create(GTA_Ast_Node * lhs, GTA_Ast_Node * rhs, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Index object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Index object to destroy.
 */
void gta_ast_node_index_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Index object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Index object to print.
 * @param indent The number of spaces to indent the output.
 */
void gta_ast_node_index_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Index object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Index object to simplify.
 * @param variable_map The variable map to use when simplifying the index
 *   operation.
 * @return The simplified GTA_Ast_Node_Index object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_index_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_Index object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Index object to walk.
 * @param callback The callback function to call for each node.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_index_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

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
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_index_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_INDEX_H
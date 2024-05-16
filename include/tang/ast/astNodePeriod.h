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
typedef struct GTA_Ast_Node_Period {
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
} GTA_Ast_Node_Period;

/**
 * Creates a new GTA_Ast_Node_Period object.
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

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_PERIOD_H